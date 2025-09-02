// Fill out your copyright notice in the Description page of Project Settings.



#include "Variant_Shooter/FPS_GAS_AbilitySystemComponent.h"
#include "FPS_GAS_GameplayTags.h"
#include "AbilitySystem/Abilities/FPS_GAS_GameplayAbility.h"

class UFPS_GAS_GameplayAbility;

void UFPS_GAS_AbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,&UFPS_GAS_AbilitySystemComponent::ClientEffectedApplied);
	const FFPS_GAS_GameplayTags& GameplayTags = FFPS_GAS_GameplayTags::Get();
}

void UFPS_GAS_AbilitySystemComponent::AddCharacterAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass,1);
		if(const UFPS_GAS_GameplayAbility* AuraAbility = Cast<UFPS_GAS_GameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->StartupInputTag);
			GiveAbility(AbilitySpec);
		}		
	}
}

void UFPS_GAS_AbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UFPS_GAS_AbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
			
		}
	}
}

void UFPS_GAS_AbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
}

void UFPS_GAS_AbilitySystemComponent::ClientEffectedApplied_Implementation(
	UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec,
	FActiveGameplayEffectHandle ActiveEffectHandle)
{
	
}
