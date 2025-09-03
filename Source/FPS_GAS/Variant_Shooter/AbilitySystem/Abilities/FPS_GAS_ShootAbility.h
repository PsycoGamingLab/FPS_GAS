// FPS_GAS_ShootAbility.h
#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AbilitySystem/Abilities/FPS_GAS_GameplayAbility.h"
#include "FPS_GAS_ShootAbility.generated.h"

class USkeletalMeshComponent;
class AShooterProjectile;

UCLASS()
class FPS_GAS_API UFPS_GAS_ShootAbility : public UFPS_GAS_GameplayAbility
{
	GENERATED_BODY()

public:
	UFPS_GAS_ShootAbility(); 
	
	// --- Config ---
	UPROPERTY(EditAnywhere, Category="Ammo")
	TSubclassOf<AShooterProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category="Aim")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditAnywhere, Category="Aim", meta=(ClampMin=0, ClampMax=1000, Units="cm"))
	float MuzzleOffset = 10.f;

	UPROPERTY(EditAnywhere, Category="Aim", meta=(ClampMin=0, ClampMax=90, Units="Degrees"))
	float AimVariance = 0.f;

	// FX (facoltativi, spostali poi in GameplayCue)
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* FiringMontage = nullptr;

protected:
	// GAS
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

private:
	// cache soft (non UPROPERTY: non vogliamo serializzare)
	TWeakObjectPtr<USkeletalMeshComponent> CachedFirstPersonMesh;
};
