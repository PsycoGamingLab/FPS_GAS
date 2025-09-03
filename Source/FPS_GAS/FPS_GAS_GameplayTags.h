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

	// Player status
	FGameplayTag Data_Damage;
	FGameplayTag State_Dead;

	// Montage
	FGameplayTag Event_HitReact;
	FGameplayTag Ability_HitReact;
	FGameplayTag Effect_HitReact;

	static FFPS_GAS_GameplayTags GameplayTags;
};
