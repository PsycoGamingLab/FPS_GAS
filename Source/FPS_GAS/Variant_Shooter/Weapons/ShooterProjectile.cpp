// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

// GAS
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "FPS_GAS_GameplayTags.h"

AShooterProjectile::AShooterProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// Replication setup so clients can see projectiles (including their own)
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);
	bNetUseOwnerRelevancy = true; // relevance follows the owning character

	// Root & collision
	RootComponent = CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));
	CollisionComponent->SetSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	// Movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->SetIsReplicated(true);

	// Legacy defaults kept (type unused by GAS)
	HitDamageType = nullptr;
}

float AShooterProjectile::GetCollisionRadius() const
{
	return CollisionComponent ? CollisionComponent->GetScaledSphereRadius() : 0.f;
}

void AShooterProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Ignore instigator if requested
	if (!bDamageOwner)
	{
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	}
}

void AShooterProjectile::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(DestructionTimer);
}

void AShooterProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// Not used with NotifyHit; left for backward compatibility if something bound it.
}

void AShooterProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	// Prevent double-processing
	if (bHit)
	{
		return;
	}
	bHit = true;

	// Stop further collision processing for this projectile
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// AI perception noise
	MakeNoise(NoiseLoudness, GetInstigator(), GetActorLocation(), NoiseRange, NoiseTag);

	// --- GAS damage (server only) ---
	if (HasAuthority() && DamageEffectClass)
	{
		if (bExplodeOnHit)
		{
			// Apply GE damage to pawns in radius
			ApplyGASRadialDamage(GetActorLocation());

			// Apply legacy physics impulses to overlapped components (no damage here)
			ExplosionCheck(GetActorLocation());
		}
		else
		{
			// Single-hit damage to the impacted actor (if allowed)
			if (Other && (bDamageOwner || Other != GetInstigator()))
			{
				const float Damage = (BaseDamage > 0.f ? BaseDamage : HitDamage);
				ApplyGASDamageToActor(Other, Hit, Damage);
			}

			// Legacy physics impulse on the impacted component
			const FVector Outward = -Hit.ImpactNormal;
			ProcessHit(Other, OtherComp, Hit.ImpactPoint, Outward);
		}
	}
	else
	{
		// Even if not authoritative or no GE set, keep physics feedback (client visuals / listen server)
		const FVector Outward = -Hit.ImpactNormal;
		ProcessHit(Other, OtherComp, Hit.ImpactPoint, Outward);
	}

	// Blueprint VFX/SFX hook
	BP_OnProjectileHit(Hit);

	// Destroy (deferred or immediate)
	if (DeferredDestructionTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			DestructionTimer, this, &AShooterProjectile::OnDeferredDestruction, DeferredDestructionTime, false);
	}
	else
	{
		Destroy();
	}
}

void AShooterProjectile::ExplosionCheck(const FVector& ExplosionCenter)
{
	// Overlap nearby actors/components (physics feedback only)
	TArray<FOverlapResult> Overlaps;

	FCollisionShape OverlapShape;
	OverlapShape.SetSphere(ExplosionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ProjectileExplosionPhysics), /*bTraceComplex*/ false);
	QueryParams.AddIgnoredActor(this);
	if (!bDamageOwner && GetInstigator())
	{
		QueryParams.AddIgnoredActor(GetInstigator());
	}

	GetWorld()->OverlapMultiByObjectType(Overlaps, ExplosionCenter, FQuat::Identity, ObjectParams, OverlapShape, QueryParams);

	TSet<AActor*> Visited;

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* A = R.GetActor();
		if (!A || Visited.Contains(A)) continue;
		Visited.Add(A);

		// Apply outward physics impulse if the component simulates
		if (UPrimitiveComponent* Comp = R.GetComponent())
		{
			if (Comp->IsSimulatingPhysics())
			{
				const FVector ExplosionDir = (A->GetActorLocation() - ExplosionCenter).GetSafeNormal();
				Comp->AddImpulseAtLocation(ExplosionDir * PhysicsForce, ExplosionCenter);
			}
		}
	}
}

void AShooterProjectile::ProcessHit(AActor* HitActor, UPrimitiveComponent* HitComp,
	const FVector& HitLocation, const FVector& HitDirection)
{
	// Physics impulse only; damage is handled via GAS elsewhere
	if (HitComp && HitComp->IsSimulatingPhysics())
	{
		HitComp->AddImpulseAtLocation(HitDirection * PhysicsForce, HitLocation);
	}
}

void AShooterProjectile::ApplyGASDamageToActor(AActor* Target, const FHitResult& Hit, float Damage)
{
	if (!Target || Damage <= 0.f || !DamageEffectClass) return;

	// Get ASCs (source = projectile instigator's ASC, target = hit actor's ASC)
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!SourceASC || !TargetASC) return;

	// Build effect context (hit info, source object for cues)
	FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	Ctx.AddHitResult(Hit);
	Ctx.AddSourceObject(this);

	// Build spec and set the SetByCaller(Data.Damage)
	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, /*Level*/1.f, Ctx);
	if (!Spec.IsValid()) return;

	Spec.Data->SetSetByCallerMagnitude(FFPS_GAS_GameplayTags::Get().Data_Damage, -FMath::Abs(Damage));

	// Apply to target
	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

void AShooterProjectile::ApplyGASRadialDamage(const FVector& Origin)
{
	if (!HasAuthority() || !DamageEffectClass) return;

	// Simple pawn-overlap in radius; you can extend to other channels if needed
	TArray<FOverlapResult> Overlaps;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ProjectileRadialDamage), /*bTraceComplex*/ false, this);
	if (!bDamageOwner && GetInstigator())
	{
		Params.AddIgnoredActor(GetInstigator());
	}

	if (!GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, ObjParams, Sphere, Params))
	{
		return;
	}

	TSet<AActor*> UniqueTargets;
	for (const FOverlapResult& R : Overlaps)
	{
		AActor* Actor = R.GetActor();
		if (!Actor || UniqueTargets.Contains(Actor)) continue;
		UniqueTargets.Add(Actor);

		// Synthetic hit for context (you can refine with a line trace)
		FHitResult DummyHit;
		DummyHit.Location = Actor->GetActorLocation();
		DummyHit.ImpactPoint = DummyHit.Location;

		const float Damage = (BaseDamage > 0.f ? BaseDamage : HitDamage);
		ApplyGASDamageToActor(Actor, DummyHit, Damage);
	}
}

void AShooterProjectile::OnDeferredDestruction()
{
	Destroy();
}
