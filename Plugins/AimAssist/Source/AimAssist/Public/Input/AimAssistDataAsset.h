#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Engine/EngineTypes.h"
#include "AimAssistDataAsset.generated.h"

/**
 * Tuning container for Aim Assist.
 * Keep all designer-facing parameters here so gameplay programmers
 * can read them without recompiling and designers can hot-swap assets.
 */
UCLASS(BlueprintType, Const)
class AIMASSIST_API UAimAssistDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	/** Master enable for this profile. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="General")
	bool bEnabled = true;

	/** Apply only when the input device is a gamepad. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="General")
	bool bOnlyGamepad = true;

	/** How often we rescan for a best target (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Targeting", meta=(ClampMin="0.01"))
	float ScanIntervalSeconds = 0.05f;

	/** Radius of the overlap sphere around the camera when looking for candidates (cm). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Targeting", meta=(ClampMin="0.0"))
	float ScanRadius = 2000.f;

	/** Hard cap on acceptable target distance (cm). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Targeting", meta=(ClampMin="0.0"))
	float MaxTargetDistance = 4500.f;

	/** Horizontal on-screen half-FOV for acquisition (deg). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Targeting", meta=(ClampMin="0.0", ClampMax="89.0"))
	float AcquisitionFOVDeg = 12.f;

	/** Object types that qualify as valid targets (e.g., Pawn). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Targeting")
	TArray<TEnumAsByte<ECollisionChannel>> TargetObjectChannels = { ECC_Pawn };

	/** Optional Line Of Sight requirement. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Targeting")
	bool bRequireLineOfSight = true;

	/** Trace channel to use for the LOS test. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Targeting")
	TEnumAsByte<ECollisionChannel> VisibilityChannel = ECC_Visibility;

	/** Weight curve by angle (deg) to prefer targets closer to the crosshair. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Targeting")
	TObjectPtr<UCurveFloat> TargetWeightCurve = nullptr;

	// ------------------------
	// Slowdown / friction
	// ------------------------

	/** Multiplier (0..1) by absolute yaw/pitch error in degrees. 1 = no slowdown, 0 = full stop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Slowdown")
	TObjectPtr<UCurveFloat> FrictionByAngle = nullptr;

	/** Distance-based multiplier (0..1). Output multiplies FrictionByAngle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Slowdown")
	TObjectPtr<UCurveFloat> FrictionByDistance = nullptr;

	/** Lower bound for slowdown multiplier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Slowdown", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MinSlowdown = 0.2f;

	// ------------------------
	// Magnetism
	// ------------------------

	/** Strength (0..1) by absolute error angle (deg). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Magnetism")
	TObjectPtr<UCurveFloat> MagnetismByAngle = nullptr;

	/** Max degrees per second we can nudge toward the target (both yaw and pitch). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Magnetism", meta=(ClampMin="0.0"))
	float MaxMagnetismDegPerSec = 30.f;

	// Rate [1/sec] controlling how fast input blends toward target direction
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Magnetism", meta=(ClampMin="0.0"))
	float MagnetismBlendSpeed = 8.f;

	// ------------------------
	// Debug
	// ------------------------

	// Debug toggle for quick on-screen visualization (lines/spheres)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Debug")
	bool bDebugDraw = false;
	

public:
	/** Stable primary asset id type. */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		static const FPrimaryAssetType Type(TEXT("AimAssistData"));
		return FPrimaryAssetId(Type, GetFName());
	}
};
