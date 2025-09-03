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

	
	GameplayTags.State_Dead  = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Dead"),
		TEXT("Character is dead")
		);

	
}
