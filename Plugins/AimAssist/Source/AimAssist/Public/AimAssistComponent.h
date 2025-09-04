#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AimAssistComponent.generated.h"

class UAimAssistDataAsset;

UCLASS(ClassGroup=(Aim), meta=(BlueprintSpawnableComponent))
class AIMASSIST_API UAimAssistComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAimAssistComponent();

	/** Enable/disable the whole feature (client-only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AimAssist")
	bool bEnabled = true;

	/** If true, aim assist is applied only when the last input device is a gamepad */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AimAssist")
	bool bOnlyGamepad = true;

	/** Draw simple debug (overlaps, chosen target, angles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AimAssist|Debug")
	bool bDrawDebug = false;

	/** Hip-fire profile (curves & params) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AimAssist|Settings")
	TObjectPtr<UAimAssistDataAsset> HipSettings = nullptr;

	/** ADS profile (optional). If null, HipSettings is used for both. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AimAssist|Settings")
	TObjectPtr<UAimAssistDataAsset> ADSSettings = nullptr;

	/** Switch between Hip and ADS at runtime */
	UFUNCTION(BlueprintCallable, Category="AimAssist")
	void SetIsADS(bool bInADS);

	/** Main entry point: filter your raw look input (X=Yaw, Y=Pitch) */
	UFUNCTION(BlueprintCallable, Category="AimAssist")
	FVector2D FilterLookInput(const FVector2D& RawLookInput, float DeltaSeconds);

	/** Degrees ↔ input units conversion (tune to your input pipeline) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AimAssist|Tuning")
	float DegreesPerInputUnitYaw = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AimAssist|Tuning")
	float DegreesPerInputUnitPitch = 1.f;

protected:
	virtual void BeginPlay() override;

	/** Returns true if owner is locally controlled (aim assist should only run locally) */
	bool IsLocalOwner() const;

	/** Get active data asset (Hip or ADS) */
	const UAimAssistDataAsset* GetActiveSettings() const;

	/** Acquire/update the best target around the camera */
	void RefreshBestTarget();

	/** Compute angular error (deg) between camera forward and target direction */
	void ComputeAngularErrorDeg(FVector& OutToTarget, float& OutYawErrorDeg, float& OutPitchErrorDeg) const;

	/** Convert degrees to input units (axis values) */
	FORCEINLINE float DegToInputYaw(float Deg) const { return DegreesPerInputUnitYaw > 0.f ? (Deg / DegreesPerInputUnitYaw) : Deg; }
	FORCEINLINE float DegToInputPitch(float Deg) const { return DegreesPerInputUnitPitch > 0.f ? (Deg / DegreesPerInputUnitPitch) : Deg; }

private:
	/** Cached state */
	bool bIsADS = false;

	/** Current chosen actor (weak ref to avoid keeping it alive) */
	TWeakObjectPtr<AActor> CurrentTarget;

	/** Timestamp for periodic scans */
	float TimeSinceLastScan = 0.f;
};
