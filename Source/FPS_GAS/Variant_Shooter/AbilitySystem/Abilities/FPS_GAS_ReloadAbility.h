#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/AbilitySystem/Abilities/FPS_GAS_GameplayAbility.h"
#include "FPS_GAS_ReloadAbility.generated.h"

class AShooterWeapon;

UCLASS()
class FPS_GAS_API UFPS_GAS_ReloadAbility : public UFPS_GAS_GameplayAbility
{
	GENERATED_BODY()

public:
	UFPS_GAS_ReloadAbility();

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

private:
	AShooterWeapon* GetCurrentWeapon(const FGameplayAbilityActorInfo* ActorInfo) const;

	// Optional montage. Safe to leave null.
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* ReloadMontage = nullptr;
};
