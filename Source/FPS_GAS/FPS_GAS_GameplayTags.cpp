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
	GameplayTags.Ability_Shoot = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Shoot"),
		TEXT("Shoot ability tag"));

	GameplayTags.Data_Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.Damage"),
		TEXT("SetByCaller damage")
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
