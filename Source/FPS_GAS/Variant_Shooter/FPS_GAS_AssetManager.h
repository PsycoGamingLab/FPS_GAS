// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "FPS_GAS_AssetManager.generated.h"

/**
 * 
 */
UCLASS()
class FPS_GAS_API UFPS_GAS_AssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	static UFPS_GAS_AssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};
