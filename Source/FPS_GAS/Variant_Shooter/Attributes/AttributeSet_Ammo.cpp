#include "AttributeSet_Ammo.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagsManager.h"

UAttributeSet_Ammo::UAttributeSet_Ammo(){}

void UAttributeSet_Ammo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, Reserve_Pistol,    COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, MaxReserve_Pistol, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, Reserve_Rifle,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, MaxReserve_Rifle,  COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, Reserve_Shells,    COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, MaxReserve_Shells, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, Clip_Pistol,       COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, MaxClip_Pistol,    COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, Clip_Rifle,        COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, MaxClip_Rifle,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, Clip_Shells,       COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Ammo, MaxClip_Shells,    COND_None, REPNOTIFY_Always);
}

#define REP_NOTIFY(name) \
void UAttributeSet_Ammo::OnRep_##name(const FGameplayAttributeData& Old){ \
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Ammo, name, Old); \
}

REP_NOTIFY(Reserve_Pistol)
REP_NOTIFY(MaxReserve_Pistol)
REP_NOTIFY(Reserve_Rifle)
REP_NOTIFY(MaxReserve_Rifle)
REP_NOTIFY(Reserve_Shells)
REP_NOTIFY(MaxReserve_Shells)

REP_NOTIFY(Clip_Pistol)
REP_NOTIFY(MaxClip_Pistol)
REP_NOTIFY(Clip_Rifle)
REP_NOTIFY(MaxClip_Rifle)
REP_NOTIFY(Clip_Shells)
REP_NOTIFY(MaxClip_Shells)

static FGameplayTag TAG_Ammo_Pistol() { return FGameplayTag::RequestGameplayTag(TEXT("Ammo.Pistol")); }
static FGameplayTag TAG_Ammo_Rifle()  { return FGameplayTag::RequestGameplayTag(TEXT("Ammo.Rifle"));  }
static FGameplayTag TAG_Ammo_Shells() { return FGameplayTag::RequestGameplayTag(TEXT("Ammo.Shells")); }

FGameplayAttribute UAttributeSet_Ammo::GetReserveAttrFromTag(FGameplayTag Tag)
{
	if (Tag == TAG_Ammo_Pistol()) return GetReserve_PistolAttribute();
	if (Tag == TAG_Ammo_Rifle())  return GetReserve_RifleAttribute();
	if (Tag == TAG_Ammo_Shells()) return GetReserve_ShellsAttribute();
	return FGameplayAttribute();
}

FGameplayAttribute UAttributeSet_Ammo::GetMaxReserveAttrFromTag(FGameplayTag Tag)
{
	if (Tag == TAG_Ammo_Pistol()) return GetMaxReserve_PistolAttribute();
	if (Tag == TAG_Ammo_Rifle())  return GetMaxReserve_RifleAttribute();
	if (Tag == TAG_Ammo_Shells()) return GetMaxReserve_ShellsAttribute();
	return FGameplayAttribute();
}

FGameplayAttribute UAttributeSet_Ammo::GetClipAttrFromTag(FGameplayTag Tag)
{
	if (Tag == TAG_Ammo_Pistol()) return GetClip_PistolAttribute();
	if (Tag == TAG_Ammo_Rifle())  return GetClip_RifleAttribute();
	if (Tag == TAG_Ammo_Shells()) return GetClip_ShellsAttribute();
	return FGameplayAttribute();
}

FGameplayAttribute UAttributeSet_Ammo::GetMaxClipAttrFromTag(FGameplayTag Tag)
{
	if (Tag == TAG_Ammo_Pistol()) return GetMaxClip_PistolAttribute();
	if (Tag == TAG_Ammo_Rifle())  return GetMaxClip_RifleAttribute();
	if (Tag == TAG_Ammo_Shells()) return GetMaxClip_ShellsAttribute();
	return FGameplayAttribute();
}
