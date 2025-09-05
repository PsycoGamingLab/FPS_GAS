#include "FPS_GAS_ReloadAbility.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Attributes/AttributeSet_Ammo.h"
#include "ShooterCharacter.h"
#include "ShooterWeapon.h"

UFPS_GAS_ReloadAbility::UFPS_GAS_ReloadAbility()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer Tags = GetAssetTags();
	Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Reload")));
	SetAssetTags(Tags);
}

AShooterWeapon* UFPS_GAS_ReloadAbility::GetCurrentWeapon(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return nullptr;
	if (AShooterCharacter* Char = Cast<AShooterCharacter>(ActorInfo->AvatarActor.Get()))
	{
		return Char->GetCurrentWeapon();
	}
	return nullptr;
}

bool UFPS_GAS_ReloadAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false;

	const AShooterWeapon* Weapon = GetCurrentWeapon(ActorInfo);
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Weapon || !ASC) return false;

	const FGameplayTag AmmoType = Weapon->GetAmmoTypeTag();
	const FGameplayAttribute ClipAttr     = UAttributeSet_Ammo::GetClipAttrFromTag(AmmoType);
	const FGameplayAttribute MaxClipAttr  = UAttributeSet_Ammo::GetMaxClipAttrFromTag(AmmoType);
	const FGameplayAttribute ReserveAttr  = UAttributeSet_Ammo::GetReserveAttrFromTag(AmmoType);
	if (!ClipAttr.IsValid() || !MaxClipAttr.IsValid() || !ReserveAttr.IsValid()) return false;

	const float Clip     = ASC->GetNumericAttribute(ClipAttr);
	const float MaxClip  = ASC->GetNumericAttribute(MaxClipAttr);
	const float Reserve  = ASC->GetNumericAttribute(ReserveAttr);

	const bool bClipFull  = Clip >= MaxClip - KINDA_SMALL_NUMBER;
	const bool bNoReserve = Reserve <= KINDA_SMALL_NUMBER;

	return (!bClipFull && !bNoReserve && MaxClip > KINDA_SMALL_NUMBER);
}

void UFPS_GAS_ReloadAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	AShooterWeapon* Weapon = GetCurrentWeapon(ActorInfo);
	if (!ASC || !Weapon)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FGameplayTag AmmoType = Weapon->GetAmmoTypeTag();
	const FGameplayAttribute ClipAttr     = UAttributeSet_Ammo::GetClipAttrFromTag(AmmoType);
	const FGameplayAttribute MaxClipAttr  = UAttributeSet_Ammo::GetMaxClipAttrFromTag(AmmoType);
	const FGameplayAttribute ReserveAttr  = UAttributeSet_Ammo::GetReserveAttrFromTag(AmmoType);
	if (!ClipAttr.IsValid() || !MaxClipAttr.IsValid() || !ReserveAttr.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const int32 Clip    = FMath::FloorToInt(ASC->GetNumericAttribute(ClipAttr));
	const int32 MaxClip = FMath::FloorToInt(ASC->GetNumericAttribute(MaxClipAttr));
	const int32 Reserve = FMath::FloorToInt(ASC->GetNumericAttribute(ReserveAttr));

	const int32 Missing = FMath::Max(0, MaxClip - Clip);
	const int32 ToLoad  = FMath::Clamp(Missing, 0, Reserve);

	if (ToLoad <= 0)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("GE_Reload_Transient"));
	GE->DurationPolicy = EGameplayEffectDurationType::Instant;

	int32 ModIdx = GE->Modifiers.AddDefaulted();
	FGameplayModifierInfo& AddToClip = GE->Modifiers[ModIdx];
	AddToClip.Attribute = ClipAttr;
	AddToClip.ModifierOp = EGameplayModOp::Additive;
	AddToClip.ModifierMagnitude = FScalableFloat(static_cast<float>(ToLoad));

	ModIdx = GE->Modifiers.AddDefaulted();
	FGameplayModifierInfo& RemoveFromReserve = GE->Modifiers[ModIdx];
	RemoveFromReserve.Attribute = ReserveAttr;
	RemoveFromReserve.ModifierOp = EGameplayModOp::Additive;
	RemoveFromReserve.ModifierMagnitude = FScalableFloat(-static_cast<float>(ToLoad));

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(GE->GetClass(), GetAbilityLevel());
	if (Spec.IsValid())
	{
		Spec.Data->Def = GE;
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}

	if (ReloadMontage)
	{
		if (AShooterCharacter* Char = Cast<AShooterCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (USkeletalMeshComponent* Mesh = Char->GetMesh())
			{
				Char->PlayAnimMontage(ReloadMontage);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
