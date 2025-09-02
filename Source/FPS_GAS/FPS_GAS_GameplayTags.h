#pragma once
#include "CoreMinimal.h"
#include "NativeGameplayTags.h"


struct FFPS_GAS_GameplayTags
{
public:
	static const FFPS_GAS_GameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	FGameplayTag Effects_HitReact;
	FGameplayTag InputTag_Weapon_Fire;

private:
	static FFPS_GAS_GameplayTags GameplayTags;
};
