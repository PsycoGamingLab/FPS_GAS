// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"

#include "AbilitySystemComponent.h"
#include "Attributes/AttributeSet_Health.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "FPS_GAS_AbilitySystemComponent.h"
#include "FPS_GAS_PlayerState.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include "FPS_GAS_GameplayTags.h"

UAbilitySystemComponent* AShooterCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init ability actor info for the server
	InitAbilityActorInfo();
	
	if (HasAuthority() && AbilitySystemComponent)
	{
		AbilitySystemComponent->SetNumericAttributeBase(
			UAttributeSet_Health::GetMaxHealthAttribute(), 100.f);
		AbilitySystemComponent->SetNumericAttributeBase(
			UAttributeSet_Health::GetHealthAttribute(),    100.f);
	}

	AddCharacterAbilities();
}

void AShooterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}


void AShooterCharacter::InitAbilityActorInfo()
{
	AFPS_GAS_PlayerState * FPS_GAS_PlayerState = GetPlayerState<AFPS_GAS_PlayerState>();
	check(FPS_GAS_PlayerState);
	FPS_GAS_PlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(FPS_GAS_PlayerState, this);
	Cast<UFPS_GAS_AbilitySystemComponent>(FPS_GAS_PlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = FPS_GAS_PlayerState->GetAbilitySystemComponent();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UAttributeSet_Health::GetHealthAttribute())
			.AddUObject(this, &AShooterCharacter::OnHealthChanged);

		float Cur = AbilitySystemComponent->GetNumericAttribute(UAttributeSet_Health::GetHealthAttribute());
		float Max = AbilitySystemComponent->GetNumericAttribute(UAttributeSet_Health::GetMaxHealthAttribute());
		if (Max <= 0.f) { Max = 1.f; }
		const float Normalized = FMath::Clamp(Cur / Max, 0.f, 1.f);

		if (HasAuthority())
		{
			Client_UpdateHUDHealth(Normalized);
			if (IsLocallyControlled())
			{
				UpdateHUDHealth_Local(Normalized); // listen server host
			}
		}
		else if (IsLocallyControlled())
		{
			UpdateHUDHealth_Local(Normalized); // client owner
		}
	}
}

void AShooterCharacter::AddCharacterAbilities()
{
	UFPS_GAS_AbilitySystemComponent* FPS_GAS_ASC = CastChecked<UFPS_GAS_AbilitySystemComponent>(AbilitySystemComponent);
	if(!HasAuthority())return;;

	FPS_GAS_ASC->AddCharacterAbilities(StartupAbilities);
}

void AShooterCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	const float NewHealth = Data.NewValue;

	float Max = 100.f;
	if (AbilitySystemComponent)
	{
		Max = AbilitySystemComponent->GetNumericAttribute(UAttributeSet_Health::GetMaxHealthAttribute());
		if (Max <= 0.f) { Max = 1.f; }
	}
	const float Normalized = FMath::Clamp(NewHealth / Max, 0.f, 1.f);

	// SERVER: send to owning client + also update locally if this is a listen server host.
	if (HasAuthority())
	{
		Client_UpdateHUDHealth(Normalized);
		if (IsLocallyControlled())
		{
			// Listen server host sees his own HUD update immediately
			UpdateHUDHealth_Local(Normalized);
		}

		if (NewHealth <= 0.f)
		{
			Die(); // server-only death flow
		}
	}
	// CLIENT OWNER: update HUD locally (no RPC)
	else if (IsLocallyControlled())
	{
		UpdateHUDHealth_Local(Normalized);
	}
}

AShooterCharacter::AShooterCharacter()
{

	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);
	}

}

void AShooterCharacter::Client_UpdateHUDHealth_Implementation(float NormalizedHealth)
{
	// Runs only on owning client.
	UpdateHUDHealth_Local(NormalizedHealth);
}

void AShooterCharacter::UpdateHUDHealth_Local(float NormalizedHealth)
{
	OnDamaged.Broadcast(NormalizedHealth);
}

// float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
// {
// 	// ignore if already dead
// 	if (CurrentHP <= 0.0f)
// 	{
// 		return 0.0f;
// 	}
//
// 	// Reduce HP
// 	CurrentHP -= Damage;
//
// 	// Have we depleted HP?
// 	if (CurrentHP <= 0.0f)
// 	{
// 		Die();
// 	}
//
// 	// update the HUD
// 	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
//
// 	return Damage;
// }

void AShooterCharacter::DoStartFiring()
{
	if (!AbilitySystemComponent) return;

	// Fire weapon with GAS
	FGameplayTagContainer ActivationTags;
	ActivationTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Shoot")));
	AbilitySystemComponent->TryActivateAbilitiesByTag(ActivationTags);
	
}

void AShooterCharacter::DoStopFiring()
{
	// stop firing the current weapon
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	// ensure we have at least two weapons two switch between
	if (OwnedWeapons.Num() > 1)
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the last weapon?
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// loop back to the beginning of the array
			WeaponIndex = 0;
		}
		else {
			// select the next weapon index
			++WeaponIndex;
		}

		// set the new weapon as current
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// activate the new weapon
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, ThirdPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// spawn the new weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// add the weapon to the owned list
			OwnedWeapons.Add(AddedWeapon);

			// if we have an existing weapon, deactivate it
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// switch to the new weapon
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// unused
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;

}

void AShooterCharacter::Die()
{
	// Only the server should run authoritative death logic
	if (!HasAuthority())
	{
		return;
	}

	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	GetCharacterMovement()->StopMovementImmediately();
	DisableInput(nullptr);

	OnBulletCountUpdated.Broadcast(0, 0);
	BP_OnDeath();

	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}

void AShooterCharacter::OnRespawn()
{
	// destroy the character to force the PC to respawn
	Destroy();
}
