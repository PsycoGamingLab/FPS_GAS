// FPS_GAS_GameplayTags.cpp
#include "FPS_GAS_GameplayTags.h"


FFPS_GAS_GameplayTags FFPS_GAS_GameplayTags::GameplayTags;


void FFPS_GAS_GameplayTags::InitializeNativeGameplayTags()
{
	// Input attributes
	GameplayTags.InputTag_Weapon_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Weapon.Fire"),
		FString("Input Tag for weapon fire")
	);

	// Ability
	GameplayTags.Ability_Reload = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Reload"),
		TEXT("Reload ability"));
	
	GameplayTags.Ability_Shoot = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Shoot"),
		TEXT("Shoot ability tag"));

	// Cooldown
	GameplayTags.Cooldown_Weapon_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Cooldown.Weapon.Fire"),
	TEXT("Blocks firing while active"));

	GameplayTags.Cooldown_Weapon_Reload = UGameplayTagsManager::Get().AddNativeGameplayTag(
FName("Cooldown.Weapon.Reload"),
TEXT("Blocks reload/other while active"));

	// Ammo
	GameplayTags.Ammo_Pistol = UGameplayTagsManager::Get().AddNativeGameplayTag(
FName("Ammo.Pistol"),
TEXT("Ammo type: pistol"));

	GameplayTags.Ammo_Rifle = UGameplayTagsManager::Get().AddNativeGameplayTag(
FName("Ammo.Rifle"),
TEXT("Ammo type: rifle"));

	GameplayTags.Ammo_Shells = UGameplayTagsManager::Get().AddNativeGameplayTag(
FName("Ammo.Shells"),
TEXT("Ammo type: shells"));

	// Data

	GameplayTags.Data_Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.Damage"),
		TEXT("SetByCaller damage")
	);

	GameplayTags.Data_Cooldown = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.Cooldown"),
		TEXT("SetByCaller: cooldown duration seconds")
	);

	GameplayTags.Data_ReloadAmount = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Data.ReloadAmount"),
	TEXT("SetByCaller: bullets to move from reserve to clip (optional)")
);


	GameplayTags.State_Dead = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Dead"),
		TEXT("Character is dead")
	);

	GameplayTags.Event_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.HitReact"),
		TEXT("Hit react event")
	);

	GameplayTags.Ability_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.HitReact"),
		TEXT("Hit react ability")
	);

	GameplayTags.Effect_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effect.HitReact"),
		TEXT("Hit react state")
	);
}
