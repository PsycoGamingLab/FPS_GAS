#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h" // Enhanced Input
#include "AimAssistInputModifier.generated.h"

class UAimAssistDataAsset;

/**
 * Enhanced Input modifier that applies aim slowdown and magnetism based on a target
 * detected near the center of the screen.
 *
 * Plug this into your Look Input Action's modifiers.
 */
UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, meta=(DisplayName="AimAssist"))
class AIMASSIST_API UAimAssistInputModifier : public UInputModifier
{
	GENERATED_BODY()

public:
	/** Data Asset for tuning (curves, distances, toggles). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	TObjectPtr<UAimAssistDataAsset> Settings;

	/** Optional runtime setter if you want to swap settings in code. */
	UFUNCTION(BlueprintCallable, Category="AimAssist")
	void SetSettings(UAimAssistDataAsset* In) { Settings = In; }

protected:
	// UInputModifier interface
	virtual FInputActionValue ModifyRaw_Implementation(
		const UEnhancedPlayerInput* PlayerInput,
		FInputActionValue CurrentValue,
		float DeltaTime) override;

private:
	bool ShouldApply(const UEnhancedPlayerInput* PlayerInput) const;
	bool FindBestTarget(const APlayerController* PC, FVector& OutTargetWorld, float& OutScreenAngleDeg) const;

	// Utility
	static float AngleBetweenViewAndPoint(const APlayerController* PC, const FVector& WorldPoint);
};
