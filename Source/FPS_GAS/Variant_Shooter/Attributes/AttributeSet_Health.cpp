#include "AttributeSet_Health.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h" 

UAttributeSet_Health::UAttributeSet_Health()
{
    InitMaxHealth(100.f);
    InitHealth(100.f);
}

void UAttributeSet_Health::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Health, Health,    COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Health, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UAttributeSet_Health::PreAttributeChange(const FGameplayAttribute& Attr, float& NewValue)
{
    Super::PreAttributeChange(Attr, NewValue);

    if (Attr == GetMaxHealthAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.f); 
        const float CurrentHealth = GetHealth();
        if (CurrentHealth > NewValue)
        {
            // Nota: il clamp vero di Health avverrà in PostGameplayEffectExecute quando arriverà il modifier
        }
    }
}

void UAttributeSet_Health::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
 
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        const float Clamped = FMath::Clamp(GetHealth(), 0.f, GetMaxHealth());
        SetHealth(Clamped);
        if (GetHealth()>0)
        {
            UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
            TargetASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.HitReact")));
        }
      
    }
    else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        const float NewMax = FMath::Max(GetMaxHealth(), 1.f);
        SetMaxHealth(NewMax);
        SetHealth(FMath::Clamp(GetHealth(), 0.f, NewMax));
    }
}

void UAttributeSet_Health::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Health, Health, OldValue);
}

void UAttributeSet_Health::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Health, MaxHealth, OldValue);
}
