#pragma once
#include "CoreMinimal.h"
#include "NativeGameplayTags.h"


struct FFPS_GAS_GameplayTags
{
public:
	static const FFPS_GAS_GameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	// Input
	FGameplayTag InputTag_Weapon_Fire;
	
	// Ability
	FGameplayTag Ability_Shoot;

	static FFPS_GAS_GameplayTags GameplayTags;
};
