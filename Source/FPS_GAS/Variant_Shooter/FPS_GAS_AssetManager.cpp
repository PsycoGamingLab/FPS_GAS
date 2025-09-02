// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Shooter/FPS_GAS_AssetManager.h"
#include "AbilitySystemGlobals.h"
#include "FPS_GAS_AbilitySystemComponent.h"
#include "FPS_GAS_GameplayTags.h"

UFPS_GAS_AssetManager& UFPS_GAS_AssetManager::Get()
{
	check(GEngine);
	UFPS_GAS_AssetManager* FPS_GAS_AssetManager = Cast<UFPS_GAS_AssetManager>(GEngine->AssetManager);
	return *FPS_GAS_AssetManager;
}


void UFPS_GAS_AssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FFPS_GAS_GameplayTags::InitializeNativeGameplayTags();

	// This is required to use Target Data
	UAbilitySystemGlobals::Get().InitGlobalData();
}