#include "Variant_Shooter/AbilitySystem/Abilities/FPS_GAS_ShootAbility.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "FPS_GAS_GameplayTags.h"
#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "ShooterProjectile.h"
#include "Engine/World.h"
#include "Attributes/AttributeSet_Ammo.h"

UFPS_GAS_ShootAbility::UFPS_GAS_ShootAbility()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	CooldownTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.Weapon.Fire"));
	ActivationBlockedTags.AddTag(CooldownTag);

	FGameplayTagContainer Tags = GetAssetTags();
	Tags.AddTag(FFPS_GAS_GameplayTags::Get().Ability_Shoot);
	SetAssetTags(Tags);
}

AShooterWeapon* UFPS_GAS_ShootAbility::GetCurrentWeapon(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return nullptr;
	if (AShooterCharacter* Char = Cast<AShooterCharacter>(ActorInfo->AvatarActor.Get()))
	{
		return Char->GetCurrentWeapon();
	}
	return nullptr;
}

float UFPS_GAS_ShootAbility::GetFireIntervalSeconds(const AShooterWeapon* Weapon) const
{
	return FireInterval > 0.f ? FireInterval : 0.1f;
}

bool UFPS_GAS_ShootAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* Info,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, Info, SourceTags, TargetTags, OptionalRelevantTags))
		return false;

	const AShooterWeapon* Weapon = GetCurrentWeapon(Info);
	const TSubclassOf<AShooterProjectile> ProjClass =
		(ProjectileClass ? ProjectileClass : (Weapon ? Weapon->GetProjectileClass() : nullptr));

	return (Weapon && ProjClass != nullptr);
}

bool UFPS_GAS_ShootAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const AShooterWeapon* Weapon = GetCurrentWeapon(ActorInfo);
	if (!ASC || !Weapon) return false;

	const FGameplayAttribute ClipAttr = UAttributeSet_Ammo::GetClipAttrFromTag(Weapon->GetAmmoTypeTag());
	if (!ClipAttr.IsValid()) return false;

	const float Clip = ASC->GetNumericAttribute(ClipAttr);
	return (Clip >= AmmoPerShot);
}

void UFPS_GAS_ShootAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const AShooterWeapon* Weapon = GetCurrentWeapon(ActorInfo);
	if (!ASC || !Weapon || AmmoPerShot <= 0) return;

	const FGameplayAttribute ClipAttr = UAttributeSet_Ammo::GetClipAttrFromTag(Weapon->GetAmmoTypeTag());
	if (!ClipAttr.IsValid()) return;

	UGameplayEffect* CostGE = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("GE_ShootAmmoCost_Transient"));
	CostGE->DurationPolicy = EGameplayEffectDurationType::Instant;

	const int32 ModIdx = CostGE->Modifiers.AddDefaulted();
	FGameplayModifierInfo& Mod = CostGE->Modifiers[ModIdx];
	Mod.Attribute = ClipAttr;
	Mod.ModifierOp = EGameplayModOp::Additive;
	Mod.ModifierMagnitude = FScalableFloat(-static_cast<float>(AmmoPerShot));

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(CostGE->GetClass(), GetAbilityLevel());
	if (Spec.IsValid())
	{
		Spec.Data->Def = CostGE;
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void UFPS_GAS_ShootAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	const AShooterWeapon* Weapon = GetCurrentWeapon(ActorInfo);
	const float Duration = GetFireIntervalSeconds(Weapon);
	if (Duration <= 0.f) return;

	if (CooldownGEClass)
	{
		FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(CooldownGEClass, GetAbilityLevel());
		if (Spec.IsValid())
		{
			const FGameplayTag DataCooldown = FGameplayTag::RequestGameplayTag(FName("Data.Cooldown"));
			Spec.Data->SetSetByCallerMagnitude(DataCooldown, Duration);
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
		return;
	}

	UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("GE_ShootCooldown_Transient")));
	GE->DurationPolicy    = EGameplayEffectDurationType::HasDuration;
	GE->DurationMagnitude = FScalableFloat(Duration);
	GE->InheritableOwnedTagsContainer.AddTag(CooldownTag);

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(GE->GetClass(), GetAbilityLevel());
	if (Spec.IsValid())
	{
		Spec.Data->Def = GE;
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void UFPS_GAS_ShootAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* Info,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerData)
{
	AShooterCharacter* Char  = Info ? Cast<AShooterCharacter>(Info->AvatarActor.Get()) : nullptr;
	AShooterWeapon*    Weapon = Char ? Char->GetCurrentWeapon() : nullptr;

	if (!Char || !Weapon)
	{
		EndAbility(Handle, Info, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, Info, ActivationInfo))
	{
		EndAbility(Handle, Info, ActivationInfo, true, true);
		return;
	}

	const FTransform MuzzleTM  = Weapon->ComputeMuzzleTransform_Server(Char);
	const FVector    MuzzleLoc = MuzzleTM.GetLocation();
	const FVector    MuzzleFwd = MuzzleTM.GetRotation().GetForwardVector();

	const FVector SpawnLoc = MuzzleLoc + (MuzzleFwd * MuzzleOffset);

	FVector EyeLoc; FRotator EyeRot;
	Char->GetActorEyesViewPoint(EyeLoc, EyeRot);

	const FVector TraceEnd = EyeLoc + (EyeRot.Vector() * 10000.f);
	FHitResult Hit;
	FCollisionQueryParams CQ(SCENE_QUERY_STAT(GAS_ShootAim), true, Char);
	GetWorld()->LineTraceSingleByChannel(Hit, EyeLoc, TraceEnd, ECC_Visibility, CQ);

	const FVector AimPoint = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
	const FVector AimDir   = (AimPoint - SpawnLoc).GetSafeNormal();
	const float   Dist     = FVector::Distance(SpawnLoc, AimPoint);

	float HalfAngleRad = 0.f;
	if (Dist > 1.f && AimVariance > 0.f)
	{
		const float SpreadCm = AimVariance;
		HalfAngleRad = FMath::Atan(SpreadCm / Dist);
		HalfAngleRad = FMath::Min(HalfAngleRad, FMath::DegreesToRadians(10.f));
	}

	const FVector  JitteredDir = (HalfAngleRad > 0.f) ? FMath::VRandCone(AimDir, HalfAngleRad) : AimDir;
	const FRotator SpawnRot    = JitteredDir.Rotation();
	const FTransform SpawnTM(SpawnRot, SpawnLoc, FVector::OneVector);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = Info->OwnerActor.Get();
	if (APawn* InstigatorPawn = Cast<APawn>(Info->AvatarActor.Get()))
	{
		SpawnParams.Instigator = InstigatorPawn;
	}

	const TSubclassOf<AShooterProjectile> ProjClass =
		(ProjectileClass ? ProjectileClass : Weapon->GetProjectileClass());

	if (ensureAlwaysMsgf(ProjClass != nullptr, TEXT("ProjectileClass not set on Ability nor Weapon")))
	{
		GetWorld()->SpawnActor<AShooterProjectile>(ProjClass, SpawnTM, SpawnParams);
	}

	if (FiringMontage && Char->GetMesh())
	{
		Char->PlayAnimMontage(FiringMontage);
	}

	EndAbility(Handle, Info, ActivationInfo, true, false);
}
