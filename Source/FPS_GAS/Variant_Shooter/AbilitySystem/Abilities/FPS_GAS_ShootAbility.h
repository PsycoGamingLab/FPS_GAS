#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AbilitySystem/Abilities/FPS_GAS_GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "FPS_GAS_ShootAbility.generated.h"

class USkeletalMeshComponent;
class AShooterProjectile;
class UGameplayEffect;
class AShooterWeapon;

UCLASS()
class FPS_GAS_API UFPS_GAS_ShootAbility : public UFPS_GAS_GameplayAbility
{
	GENERATED_BODY()

public:
	UFPS_GAS_ShootAbility();

	// Projectile / Aim
	UPROPERTY(EditAnywhere, Category="Ammo")
	TSubclassOf<AShooterProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category="Aim")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditAnywhere, Category="Aim", meta=(ClampMin=0, ClampMax=1000, Units="cm"))
	float MuzzleOffset = 10.f;

	UPROPERTY(EditAnywhere, Category="Aim", meta=(ClampMin=0, ClampMax=90, Units="Degrees"))
	float AimVariance = 0.f;

	// Optional animation
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* FiringMontage = nullptr;

	// Ammo / Cost
	UPROPERTY(EditAnywhere, Category="Ammo")
	int32 AmmoPerShot = 1;

	// Cooldown
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cooldown")
	FGameplayTag CooldownTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cooldown")
	TSubclassOf<UGameplayEffect> CooldownGEClass;

	UPROPERTY(EditAnywhere, Category="Cooldown", meta=(ClampMin="0.01"))
	float FireInterval = 0.1f;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

private:
	AShooterWeapon* GetCurrentWeapon(const FGameplayAbilityActorInfo* ActorInfo) const;
	float GetFireIntervalSeconds(const AShooterWeapon* Weapon) const;

	TWeakObjectPtr<USkeletalMeshComponent> CachedFirstPersonMesh;
};
