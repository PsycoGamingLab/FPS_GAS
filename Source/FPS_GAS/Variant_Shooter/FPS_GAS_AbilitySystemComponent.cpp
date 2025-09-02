// Fill out your copyright notice in the Description page of Project Settings.



#include "Variant_Shooter/FPS_GAS_AbilitySystemComponent.h"
#include "FPS_GAS_GameplayTags.h"

void UFPS_GAS_AbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,&UFPS_GAS_AbilitySystemComponent::ClientEffectedApplied);
	const FFPS_GAS_GameplayTags& GameplayTags = FFPS_GAS_GameplayTags::Get();
}

void UFPS_GAS_AbilitySystemComponent::ClientEffectedApplied_Implementation(
	UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec,
	FActiveGameplayEffectHandle ActiveEffectHandle)
{
	
}
