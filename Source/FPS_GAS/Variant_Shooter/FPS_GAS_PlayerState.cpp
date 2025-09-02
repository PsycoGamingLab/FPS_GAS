// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS_GAS_PlayerState.h"
#include "FPS_GAS_AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

AFPS_GAS_PlayerState::AFPS_GAS_PlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UFPS_GAS_AbilitySystemComponent>("AbilitySystem_Component");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	SetNetUpdateFrequency(100.f);
	
}

void AFPS_GAS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPS_GAS_PlayerState,level);
}

UAbilitySystemComponent* AFPS_GAS_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AFPS_GAS_PlayerState::OnRep_Level(int32 OldLevel)
{
	
}
