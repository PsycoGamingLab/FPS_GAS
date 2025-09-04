// ShooterPlayerController.cpp

#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "FPS_GAS.h"
#include "FPS_GAS_AbilitySystemComponent.h"
#include "Input/FPS_GAS_InputComponent.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "FPS_GAS_GameplayTags.h"
// NEW: include the health attribute set so we can read attributes
#include "Attributes/AttributeSet_Health.h"

AShooterPlayerController::AShooterPlayerController()
{
	bReplicates = true;
}

UFPS_GAS_AbilitySystemComponent* AShooterPlayerController::GetASC()
{
	if (FPS_GAS_AbilitySystemComponent == nullptr)
	{
		FPS_GAS_AbilitySystemComponent = Cast<UFPS_GAS_AbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()));
	}
	return FPS_GAS_AbilitySystemComponent;
}

void AShooterPlayerController::SetPawn(APawn* InPawn)
{
	// Cache the old pawn before the swap
	APawn* PrevPawn = GetPawn();

	// Do the engine swap
	Super::SetPawn(InPawn);

	// Client-only UI (re)binding
	if (IsLocalPlayerController())
	{
		// Unhook from previous pawn (if any) to avoid double binds
		UnbindFromPawn(PrevPawn);

		// Bind to the new pawn to receive health/ammo updates
		BindToPawn(InPawn);
	}
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Only create UI on the owning client
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
			if (MobileControlsWidget)
			{
				MobileControlsWidget->AddToPlayerScreen(0);
			}
			else
			{
				UE_LOG(LogFPS_GAS, Error, TEXT("Could not spawn mobile controls widget."));
			}
		}

		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);
		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);

			// Bind immediately to the current pawn (if any)
			BindToPawn(GetPawn());
		}
		else
		{
			UE_LOG(LogFPS_GAS, Error, TEXT("Could not spawn bullet counter widget."));
		}
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		// Ability input setup (unchanged)
		UFPS_GAS_InputComponent* IC = NewObject<UFPS_GAS_InputComponent>(this);
		IC->RegisterComponent();
		PushInputComponent(IC);

		IC->BindAbilityActions(InputConfig, this,
			&ThisClass::AbilityInputTagPressed,
			&ThisClass::AbilityInputTagReleased,
			&ThisClass::AbilityInputTagHeld);

		// Mapping contexts (unchanged)
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Server-only logic is fine here (tagging, respawn hooks), but bindings for UI must be client-side.
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// Add the player tag (typically server-side concern)
		ShooterCharacter->Tags.Add(PlayerPawnTag);
	}

	// Ensure the local client also binds to the newly possessed pawn
	if (IsLocalPlayerController())
	{
		BindToPawn(InPawn);
	}
}

// === NEW: binding helpers ===

void AShooterPlayerController::HandlePossessedPawnChanged(APawn* PrevPawn, APawn* InPawn)
{
	UnbindFromPawn(PrevPawn);
	BindToPawn(InPawn);
}

void AShooterPlayerController::BindToPawn(APawn* InPawn)
{
	if (!IsLocalPlayerController() || !BulletCounterUI) return;

	AShooterCharacter* Char = Cast<AShooterCharacter>(InPawn);
	if (!Char) return;

	// Avoid double-binding to the same pawn
	if (BoundCharacter.Get() == Char) return;

	// Clean old bindings if any
	if (AShooterCharacter* Old = BoundCharacter.Get())
	{
		Old->OnBulletCountUpdated.RemoveDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		Old->OnDamaged.RemoveDynamic(this, &AShooterPlayerController::OnPawnDamaged);
	}

	BoundCharacter = Char;

	// Bind the character delegates to PC handlers (which update the widget)
	Char->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
	Char->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);

	// Push initial health to the widget (read from ASC if present)
	float Max = 100.f, Cur = 100.f;
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Char))
	{
		Max = ASC->GetNumericAttribute(UAttributeSet_Health::GetMaxHealthAttribute());
		Cur = ASC->GetNumericAttribute(UAttributeSet_Health::GetHealthAttribute());
		if (Max <= 0.f) { Max = 1.f; }
	}
	const float Normalized = FMath::Clamp(Cur / Max, 0.f, 1.f);
	OnPawnDamaged(Normalized); // drive the same path the event would use
}

void AShooterPlayerController::UnbindFromPawn(APawn* InPawn)
{
	AShooterCharacter* Old = Cast<AShooterCharacter>(InPawn);
	if (!Old) return;

	Old->OnBulletCountUpdated.RemoveDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
	Old->OnDamaged.RemoveDynamic(this, &AShooterPlayerController::OnPawnDamaged);
	BoundCharacter.Reset();
}

// === existing code below stays the same ===

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	// (Template respawn logic - not MP safe, but fine for your current test)
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];
		const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

		if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
		{
			Possess(RespawnedCharacter);
		}
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

void AShooterPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagPressed(InputTag);
}

void AShooterPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagReleased(InputTag);
}

void AShooterPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagHeld(InputTag);
}
