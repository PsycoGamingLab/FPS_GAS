// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Shooter/AbilitySystem/Abilities/FPS_GAS_ShootAbility.h"

#include "AbilitySystemComponent.h"
#include "ShooterCharacter.h"
#include "ShooterProjectile.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"


void UFPS_GAS_ShootAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo,
                                            const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (AShooterCharacter* Character = Cast<AShooterCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (Character->GetCurrentWeapon() == nullptr || Character->GetCurrentWeapon()->GetFirstPersonMesh() == nullptr)
		{
		
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
		USkeletalMeshComponent* Mesh = Character->GetCurrentWeapon()->GetFirstPersonMesh();
		const FTransform MuzzleTransform = Mesh->GetSocketTransform(FName("Muzzle"));
		FireProjectile(MuzzleTransform.GetLocation());
	}
}

void UFPS_GAS_ShootAbility::FireProjectile(const FVector& TargetLocation)
{
	// get the projectile transform
	FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);

	// spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetActorInfo().AbilitySystemComponent->GetAvatarActor();
	SpawnParams.Instigator = nullptr;

	AShooterProjectile* Projectile = GetWorld()->SpawnActor<AShooterProjectile>(
		ProjectileClass, ProjectileTransform, SpawnParams);

	// play the firing montage
	//WeaponOwner->PlayFiringMontage(FiringMontage);

	// add recoil
	//WeaponOwner->AddWeaponRecoil(FiringRecoil);

	// consume bullets
	--CurrentBullets;

	// if the clip is depleted, reload it
	if (CurrentBullets <= 0)
	{
		CurrentBullets = MagazineSize;
	}

	// update the weapon HUD
//	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
}

FTransform UFPS_GAS_ShootAbility::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	if (FirstPersonMesh == nullptr) return FTransform();
	// find the muzzle location
	const FVector MuzzleLoc = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);

	// calculate the spawn location ahead of the muzzle
	const FVector SpawnLoc = MuzzleLoc + ((TargetLocation - MuzzleLoc).GetSafeNormal() * MuzzleOffset);

	// find the aim rotation vector while applying some variance to the target 
	const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(
		SpawnLoc, TargetLocation + (UKismetMathLibrary::RandomUnitVector() * AimVariance));

	// return the built transform
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}
