#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet_Ammo.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class FPS_GAS_API  UAttributeSet_Ammo : public UAttributeSet
{
    GENERATED_BODY()
public:
    UAttributeSet_Ammo();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ----- Pistol -----
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Reserve_Pistol, Category="Ammo|Reserve")
    FGameplayAttributeData Reserve_Pistol;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, Reserve_Pistol)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxReserve_Pistol, Category="Ammo|Reserve")
    FGameplayAttributeData MaxReserve_Pistol;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, MaxReserve_Pistol)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Clip_Pistol, Category="Ammo|Clip")
    FGameplayAttributeData Clip_Pistol;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, Clip_Pistol)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxClip_Pistol, Category="Ammo|Clip")
    FGameplayAttributeData MaxClip_Pistol;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, MaxClip_Pistol)

    // ----- Rifle -----
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Reserve_Rifle, Category="Ammo|Reserve")
    FGameplayAttributeData Reserve_Rifle;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, Reserve_Rifle)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxReserve_Rifle, Category="Ammo|Reserve")
    FGameplayAttributeData MaxReserve_Rifle;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, MaxReserve_Rifle)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Clip_Rifle, Category="Ammo|Clip")
    FGameplayAttributeData Clip_Rifle;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, Clip_Rifle)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxClip_Rifle, Category="Ammo|Clip")
    FGameplayAttributeData MaxClip_Rifle;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, MaxClip_Rifle)

     // ----- Shells -----
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Reserve_Shells, Category="Ammo|Reserve")
    FGameplayAttributeData Reserve_Shells;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, Reserve_Shells)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxReserve_Shells, Category="Ammo|Reserve")
    FGameplayAttributeData MaxReserve_Shells;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, MaxReserve_Shells)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Clip_Shells, Category="Ammo|Clip")
    FGameplayAttributeData Clip_Shells;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, Clip_Shells)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxClip_Shells, Category="Ammo|Clip")
    FGameplayAttributeData MaxClip_Shells;
    ATTRIBUTE_ACCESSORS(UAttributeSet_Ammo, MaxClip_Shells)

	UFUNCTION() void OnRep_Reserve_Rifle(const FGameplayAttributeData& Old);
	UFUNCTION() void OnRep_MaxReserve_Rifle(const FGameplayAttributeData& Old);
	UFUNCTION() void OnRep_Clip_Rifle(const FGameplayAttributeData& Old);
	UFUNCTION() void OnRep_MaxClip_Rifle(const FGameplayAttributeData& Old);
    
	UFUNCTION() void OnRep_Reserve_Pistol(const FGameplayAttributeData& Old);
	UFUNCTION() void OnRep_MaxReserve_Pistol(const FGameplayAttributeData& Old);
	UFUNCTION() void OnRep_Clip_Pistol(const FGameplayAttributeData& Old);
	UFUNCTION() void OnRep_MaxClip_Pistol(const FGameplayAttributeData& Old);
    
    UFUNCTION() void OnRep_Reserve_Shells(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_MaxReserve_Shells(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_Clip_Shells(const FGameplayAttributeData& Old);
	UFUNCTION() void OnRep_MaxClip_Shells(const FGameplayAttributeData& Old);
    

	static FGameplayAttribute GetReserveAttrFromTag(FGameplayTag AmmoTag);
	static FGameplayAttribute GetMaxReserveAttrFromTag(FGameplayTag AmmoTag);
	static FGameplayAttribute GetClipAttrFromTag(FGameplayTag AmmoTag);
	static FGameplayAttribute GetMaxClipAttrFromTag(FGameplayTag AmmoTag);

};

