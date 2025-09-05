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

	// Cooldown
	FGameplayTag Cooldown_Weapon_Fire;
	FGameplayTag Cooldown_Weapon_Reload;
	
	// Ability
	FGameplayTag Ability_Shoot;
	FGameplayTag Ability_HitReact;
	FGameplayTag Ability_Reload;

	// Weapon
	FGameplayTag Ability_Weapon_Fire;
	FGameplayTag Ability_Weapon_Reload;
	
	// Player status
	FGameplayTag Data_Damage;
	FGameplayTag State_Dead;

	// Montage
	FGameplayTag Event_HitReact;

	// Effect
	FGameplayTag Effect_HitReact;

	// Ammo
	FGameplayTag Ammo_Pistol;
	FGameplayTag Ammo_Rifle;
	FGameplayTag Ammo_Shells;

	// Data
	FGameplayTag Data_Cooldown;
	FGameplayTag Data_ReloadAmount;

	static FFPS_GAS_GameplayTags GameplayTags;
};
