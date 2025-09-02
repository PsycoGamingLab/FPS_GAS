// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "FPS_GAS_InputConfig.h"
#include "FPS_GAS_InputComponent.generated.h"

/**
 * 
 */
UCLASS()
class FPS_GAS_API UFPS_GAS_InputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
	public:
    	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
    	void BindAbilityActions(const UFPS_GAS_InputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
	
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UFPS_GAS_InputComponent::BindAbilityActions(const UFPS_GAS_InputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);
 
	for (const FFPS_GAS_InputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}
 
			if (ReleasedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}
 			
			if (HeldFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}