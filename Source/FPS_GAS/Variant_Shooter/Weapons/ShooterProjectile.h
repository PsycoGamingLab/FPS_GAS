// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class ACharacter;
class UPrimitiveComponent;
class UGameplayEffect;   // forward for TSubclassOf<UGameplayEffect>
class UDamageType;       // forward for TSubclassOf<UDamageType>

/**
 *  Simple projectile class for a first person shooter game.
 *  - Uses GAS to apply damage in NotifyHit (server-side).
 *  - Keeps legacy physics impulses and Blueprint hit hook.
 */
UCLASS(abstract)
class FPS_GAS_API AShooterProjectile : public AActor
{
	GENERATED_BODY()
	
	/** Collision for the projectile (root) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionComponent;

	/** Movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	/** GameplayEffect to apply damage (expects: Modifier on Health = -SetByCaller(Data.Damage)) */
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** Base damage fed into SetByCaller(Data.Damage). If 0, falls back to HitDamage. */
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float BaseDamage = 25.f;

protected:

	/** AI Perception noise on hit */
	UPROPERTY(EditAnywhere, Category="Projectile|Noise", meta = (ClampMin = 0, ClampMax = 100))
	float NoiseLoudness = 3.0f;

	/** AI Perception noise range on hit */
	UPROPERTY(EditAnywhere, Category="Projectile|Noise", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float NoiseRange = 3000.0f;

	/** AI Perception noise tag on hit */
	UPROPERTY(EditAnywhere, Category="Noise")
	FName NoiseTag = FName("Projectile");

	/** Physics impulse magnitude applied to simulating components on hit */
	UPROPERTY(EditAnywhere, Category="Projectile|Hit", meta = (ClampMin = 0, ClampMax = 50000))
	float PhysicsForce = 100.0f;

	/** Legacy damage (kept for backwards compatibility; unused by GAS unless BaseDamage==0) */
	UPROPERTY(EditAnywhere, Category="Projectile|Hit", meta = (ClampMin = 0, ClampMax = 100))
	float HitDamage = 25.0f;

	/** Legacy damage type (kept for backwards compatibility) */
	UPROPERTY(EditAnywhere, Category="Projectile|Hit")
	TSubclassOf<UDamageType> HitDamageType;

	/** If true, projectile can damage its instigator */
	UPROPERTY(EditAnywhere, Category="Projectile|Hit")
	bool bDamageOwner = false;

	/** If true, the projectile explodes and damages actors in radius */
	UPROPERTY(EditAnywhere, Category="Projectile|Explosion")
	bool bExplodeOnHit = false;

	/** Max distance for actors to be affected by explosion damage */
	UPROPERTY(EditAnywhere, Category="Projectile|Explosion", meta = (ClampMin = 0, ClampMax = 5000, Units = "cm"))
	float ExplosionRadius = 500.0f;

	/** True after first valid hit to avoid double-processing */
	bool bHit = false;

	/** Delay before destroying the projectile after a hit */
	UPROPERTY(EditAnywhere, Category="Projectile|Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float DeferredDestructionTime = 5.0f;

	/** Timer for deferred destruction */
	FTimerHandle DestructionTimer;

	/** Kept for compatibility if you bound OnComponentHit elsewhere (not used with NotifyHit) */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			   FVector NormalImpulse, const FHitResult& Hit);

public:
	AShooterProjectile();

	float GetCollisionRadius() const;

protected:
	// AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Main collision handler: uses GAS to apply damage (server-side) and keeps physics/FX flow */
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

protected:
	/** Overlap actors in explosion radius and apply physics impulses (legacy physics only) */
	void ExplosionCheck(const FVector& ExplosionCenter);

	/** Apply physics impulse to the impacted component; no damage here (GAS handles it) */
	void ProcessHit(AActor* HitActor, UPrimitiveComponent* HitComp, const FVector& HitLocation, const FVector& HitDirection);

	/** GAS: apply single-target damage via GameplayEffect */
	void ApplyGASDamageToActor(AActor* Target, const FHitResult& Hit, float Damage);

	/** GAS: apply radial damage via GameplayEffect to pawns within ExplosionRadius */
	void ApplyGASRadialDamage(const FVector& Origin);

	/** Blueprint hook for VFX/SFX on hit */
	UFUNCTION(BlueprintImplementableEvent, Category="Projectile", meta = (DisplayName = "On Projectile Hit"))
	void BP_OnProjectileHit(const FHitResult& Hit);

	/** Deferred destruction callback */
	void OnDeferredDestruction();
};
