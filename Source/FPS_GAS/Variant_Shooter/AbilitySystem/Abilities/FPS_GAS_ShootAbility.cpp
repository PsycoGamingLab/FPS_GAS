#include "Variant_Shooter/AbilitySystem/Abilities/FPS_GAS_ShootAbility.h"

#include "AbilitySystemComponent.h"
#include "FPS_GAS_GameplayTags.h"
#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "ShooterProjectile.h"
#include "Engine/World.h"

UFPS_GAS_ShootAbility::UFPS_GAS_ShootAbility()
{    
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

    FGameplayTagContainer Tags = GetAssetTags();
    Tags.AddTag(FFPS_GAS_GameplayTags::Get().Ability_Shoot);
    SetAssetTags(Tags);
}

bool UFPS_GAS_ShootAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, Info, SourceTags, TargetTags, OptionalRelevantTags))
        return false;

    AShooterCharacter* Char = Info ? Cast<AShooterCharacter>(Info->AvatarActor.Get()) : nullptr;
    const AShooterWeapon*    Weapon = Char ? Char->GetCurrentWeapon() : nullptr;
    
    return (Weapon && Weapon->GetProjectileClass() != nullptr);
}

void UFPS_GAS_ShootAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerData)
{
    AShooterCharacter* Char = Info ? Cast<AShooterCharacter>(Info->AvatarActor.Get()) : nullptr;
    AShooterWeapon*    Weapon = Char ? Char->GetCurrentWeapon() : nullptr;

    if (!Char || !Weapon || !Weapon->GetProjectileClass())
    {
        EndAbility(Handle, Info, ActivationInfo, /*bReplicateEnd*/true, /*bWasCancelled*/true);
        return;
    }
    
    const FTransform MuzzleTM   = Weapon->ComputeMuzzleTransform_Server(Char);
    const FVector    MuzzleLoc  = MuzzleTM.GetLocation();
    const FVector    MuzzleFwd  = MuzzleTM.GetRotation().GetForwardVector();
   
    const FVector SpawnLoc = MuzzleLoc + (MuzzleFwd * Weapon->GetMuzzleOffset());
    
    FVector EyeLoc; FRotator EyeRot;
    Char->GetActorEyesViewPoint(EyeLoc, EyeRot);

    const FVector TraceEnd = EyeLoc + (EyeRot.Vector() * 10000.f);
    FHitResult Hit;
    FCollisionQueryParams CQ(SCENE_QUERY_STAT(GAS_ShootAim), /*bTraceComplex*/true, Char);
    GetWorld()->LineTraceSingleByChannel(Hit, EyeLoc, TraceEnd, ECC_Visibility, CQ);

    const FVector AimPoint = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
    
    const FVector AimDir = (AimPoint - SpawnLoc).GetSafeNormal();
    const float   Dist   = FVector::Distance(SpawnLoc, AimPoint);

    float HalfAngleRad = 0.f;
    if (Dist > 1.f)
    {
        const float SpreadCm = Weapon->GetAimVariance(); 
        HalfAngleRad = FMath::Atan(SpreadCm / Dist);
        HalfAngleRad = FMath::Min(HalfAngleRad, FMath::DegreesToRadians(10.f)); 
    }

    const FVector  JitteredDir = (HalfAngleRad > 0.f) ? FMath::VRandCone(AimDir, HalfAngleRad) : AimDir;
    const FRotator SpawnRot    = JitteredDir.Rotation();
    const FTransform SpawnTM(SpawnRot, SpawnLoc, FVector::OneVector);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = Info->OwnerActor.Get();
    if (APawn* InstigatorPawn = Cast<APawn>(Info->AvatarActor.Get()))
    {
        SpawnParams.Instigator = InstigatorPawn;
    }

    AShooterProjectile* Proj = GetWorld()->SpawnActor<AShooterProjectile>(
        Weapon->GetProjectileClass(), SpawnTM, SpawnParams);
    
    EndAbility(Handle, Info, ActivationInfo, /*bReplicateEnd*/true, /*bWasCancelled*/false);
}
