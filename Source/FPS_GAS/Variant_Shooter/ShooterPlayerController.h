// ShooterPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Input/FPS_GAS_InputConfig.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class AShooterCharacter;
class UShooterBulletCounterUI;
class UFPS_GAS_AbilitySystemComponent;

UCLASS()
class FPS_GAS_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterPlayerController();

	UFPS_GAS_AbilitySystemComponent* GetASC();

protected:
	// Rebind UI delegates whenever our pawn changes (runs on client as well)
	virtual void SetPawn(APawn* InPawn) override;
	
	// --- UI widgets & mappings (unchanged from your version) ---
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	TObjectPtr<UUserWidget> MobileControlsWidget;

	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	UPROPERTY(EditAnywhere, Category="Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;

private:
	UPROPERTY(EditAnywhere,Category="Input")
	TObjectPtr<UInputMappingContext> FPS_GAS_Context;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UFPS_GAS_InputConfig> InputConfig;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY()
	TObjectPtr<UFPS_GAS_AbilitySystemComponent> FPS_GAS_AbilitySystemComponent;

	// --- NEW: track current bound character and handle (un)binding ---
protected:
	/** Cached character we are currently bound to (for clean unbinding). */
	UPROPERTY()
	TWeakObjectPtr<AShooterCharacter> BoundCharacter;

	/** Called whenever the possessed pawn changes (fires on client too). */
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* PrevPawn, APawn* InPawn);

	/** Bind our UI delegates to a new pawn (client-side). */
	void BindToPawn(APawn* InPawn);

	/** Unbind previously bound delegates (client-side). */
	void UnbindFromPawn(APawn* InPawn);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	UFUNCTION()
	void OnPawnDamaged(float LifePercent);
};
