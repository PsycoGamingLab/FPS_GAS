// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "Attributes/AttributeSet_Health.h"
#include "FPS_GAS_PlayerState.generated.h"


class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class FPS_GAS_API AFPS_GAS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFPS_GAS_PlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Attributes
	UAttributeSet_Health* GetHealthSet() const { return HealthSet; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

private:

	UPROPERTY(VisibleAnywhere,ReplicatedUsing=OnRep_Level)
	int32 level = 1;

	UPROPERTY()
	TObjectPtr<UAttributeSet_Health> HealthSet;

	UFUNCTION()
	void OnRep_Level(int32 OldLevel);
};


