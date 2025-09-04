#include "AimAssistComponent.h"
#include "Input/AimAssistDataAsset.h"

#include "Camera/PlayerCameraManager.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

// ----------------------------------------
// Local helpers
// ----------------------------------------

// Build object query params from the DataAsset's channel array.
// Falls back to ECC_Pawn if none is set.
static FCollisionObjectQueryParams MakeAimAssistObjParams(const UAimAssistDataAsset* Settings)
{
	FCollisionObjectQueryParams Params;
	if (Settings && Settings->TargetObjectChannels.Num() > 0)
	{
		for (const TEnumAsByte<ECollisionChannel> Chan : Settings->TargetObjectChannels)
		{
			Params.AddObjectTypesToQuery(Chan);
		}
	}
	else
	{
		Params.AddObjectTypesToQuery(ECC_Pawn); // sensible default
	}
	return Params;
}

// ----------------------------------------
// Component
// ----------------------------------------

UAimAssistComponent::UAimAssistComponent()
{
	// We process inside the input path; no need to tick by default.
	PrimaryComponentTick.bCanEverTick = false;
}

void UAimAssistComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAimAssistComponent::SetIsADS(bool bInADS)
{
	bIsADS = bInADS;
}

bool UAimAssistComponent::IsLocalOwner() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	if (const APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		return Pawn->IsLocallyControlled();
	}
	if (const AController* Controller = Cast<AController>(OwnerActor))
	{
		return Controller->IsLocalController();
	}

	// Fallback: allow if we find a local PC in PIE
	if (const UWorld* World = GetWorld())
	{
		return (World->GetFirstPlayerController() != nullptr);
	}
	return false;
}

const UAimAssistDataAsset* UAimAssistComponent::GetActiveSettings() const
{
	// ADS takes precedence if available and ADS flag is on
	if (bIsADS && ADSSettings)
	{
		return ADSSettings;
	}
	// Fallback to hip settings, or ADS if hip is not set
	return HipSettings ? HipSettings : ADSSettings;
}

void UAimAssistComponent::ComputeAngularErrorDeg(FVector& OutToTarget, float& OutYawErrorDeg, float& OutPitchErrorDeg) const
{
	OutYawErrorDeg = 0.f;
	OutPitchErrorDeg = 0.f;
	OutToTarget = FVector::ZeroVector;

	const UAimAssistDataAsset* Settings = GetActiveSettings();
	if (!Settings)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController())
	                                        : Cast<APlayerController>(GetOwner());
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	const FVector  CamLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FRotator CamRot = PC->PlayerCameraManager->GetCameraRotation();

	const AActor* Target = CurrentTarget.Get();
	if (!Target)
	{
		return;
	}

	const FVector TargetLoc = Target->GetActorLocation();
	OutToTarget = (TargetLoc - CamLoc);

	// Delta between where we look and where the target is.
	const FVector  DirToTarget = OutToTarget.GetSafeNormal();
	const FRotator TargetRot   = DirToTarget.Rotation();
	const FRotator Delta       = UKismetMathLibrary::NormalizedDeltaRotator(TargetRot, CamRot);

	OutYawErrorDeg   = Delta.Yaw;
	OutPitchErrorDeg = Delta.Pitch;
}

void UAimAssistComponent::RefreshBestTarget()
{
	const UAimAssistDataAsset* Settings = GetActiveSettings();
	if (!Settings)
	{
		CurrentTarget.Reset();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		CurrentTarget.Reset();
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController())
	                                        : Cast<APlayerController>(GetOwner());
	if (!PC || !PC->PlayerCameraManager)
	{
		CurrentTarget.Reset();
		return;
	}

	const FVector  CamLoc     = PC->PlayerCameraManager->GetCameraLocation();
	const FRotator CamRot     = PC->PlayerCameraManager->GetCameraRotation();
	const FVector  CamForward = CamRot.Vector();

	// Collect candidates via an object-type overlap around the camera
	TArray<FOverlapResult> Overlaps;
	const FCollisionObjectQueryParams ObjParams = MakeAimAssistObjParams(Settings);

	FCollisionQueryParams QP(SCENE_QUERY_STAT(AimAssistOverlap), /*bTraceComplex*/ false);
	QP.AddIgnoredActor(GetOwner());

	const float Radius = Settings->MaxTargetDistance; // scan up to max distance
	const bool bAny = World->OverlapMultiByObjectType(
		Overlaps,
		CamLoc,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(Radius),
		QP
	);

	if (!bAny)
	{
		CurrentTarget.Reset();
		return;
	}

	AActor* BestActor = nullptr;
	float   BestScore = -FLT_MAX;

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* A = R.GetActor();
		if (!A || A == GetOwner())
		{
			continue;
		}

		// Distance filter
		const float Dist = FVector::Dist(CamLoc, A->GetActorLocation());
		if (Dist > Settings->MaxTargetDistance)
		{
			continue;
		}

		// Optional LOS test
		if (Settings->bRequireLineOfSight)
		{
			FHitResult Hit;
			FCollisionQueryParams VisParams(SCENE_QUERY_STAT(AimAssistLOS), /*bTraceComplex*/ true);
			VisParams.AddIgnoredActor(GetOwner());

			const bool bBlocked = World->LineTraceSingleByChannel(
				Hit,
				CamLoc,
				A->GetActorLocation(),
				Settings->VisibilityChannel,
				VisParams
			);

			if (bBlocked && Hit.GetActor() != A)
			{
				// Occluded by something else
				continue;
			}
		}

		// Angle from camera forward
		const FVector To       = (A->GetActorLocation() - CamLoc).GetSafeNormal();
		const float   Cos      = FVector::DotProduct(CamForward, To);
		const float   AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Cos, -1.f, 1.f)));

		if (AngleDeg > Settings->AcquisitionFOVDeg)
		{
			continue; // outside angular gate
		}

		// Basic weight: prefer smaller angle and slightly prefer closer targets.
		const float AngleWeight =
			(Settings->AcquisitionFOVDeg > 0.f)
				? (1.f - FMath::Clamp(AngleDeg / Settings->AcquisitionFOVDeg, 0.f, 1.f))
				: 1.f;

		// Distance preference (small effect): closer => larger
		const float DistWeight = FMath::GetRangePct(/*RangeStart*/ Settings->MaxTargetDistance, /*RangeEnd*/ 0.f, Dist);

		const float TotalWeight = AngleWeight + 0.1f * DistWeight;

		if (TotalWeight > BestScore)
		{
			BestScore = TotalWeight;
			BestActor = A;
		}
	}

	CurrentTarget = BestActor;

	const bool bDebug = bDrawDebug || (Settings && Settings->bDebugDraw);
	if (bDebug)
	{
		DrawDebugSphere(GetWorld(), CamLoc, Radius, 16, FColor::Cyan, false, 0.05f);
		if (AActor* T = CurrentTarget.Get())
		{
			DrawDebugLine(GetWorld(), CamLoc, T->GetActorLocation(), FColor::Green, false, 0.05f, 0, 1.5f);
		}
	}
}

FVector2D UAimAssistComponent::FilterLookInput(const FVector2D& RawLookInput, float DeltaSeconds)
{
	// Early outs: component disabled or not locally owned
	if (!bEnabled || !IsLocalOwner())
	{
		return RawLookInput;
	}

	const UAimAssistDataAsset* Settings = GetActiveSettings();
	if (!Settings)
	{
		return RawLookInput;
	}

	// Optional: apply only on gamepad per settings (replace with your real device gate if you have one).
	if (Settings->bOnlyGamepad && !bOnlyGamepad)
	{
		// If you track last input device, gate here.
	}

	// Refresh target periodically (fixed interval here to avoid depending on a DA field)
	TimeSinceLastScan += DeltaSeconds;
	if (TimeSinceLastScan >= 0.05f) // ~20 Hz
	{
		TimeSinceLastScan = 0.f;
		RefreshBestTarget();
	}

	// No target? return input as-is
	if (!CurrentTarget.IsValid())
	{
		return RawLookInput;
	}

	// Compute angular error to target
	FVector ToTarget;
	float YawErrDeg = 0.f, PitchErrDeg = 0.f;
	ComputeAngularErrorDeg(ToTarget, YawErrDeg, PitchErrDeg);

	const float AbsYawErr   = FMath::Abs(YawErrDeg);
	const float AbsPitchErr = FMath::Abs(PitchErrDeg);

	// Query current distance (for distance-based slowdown)
	float DistanceCm = 0.f;
	{
		const APawn* OwnerPawn = Cast<APawn>(GetOwner());
		const APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController())
		                                        : Cast<APlayerController>(GetOwner());
		if (PC && PC->PlayerCameraManager && CurrentTarget.IsValid())
		{
			DistanceCm = FVector::Dist(PC->PlayerCameraManager->GetCameraLocation(), CurrentTarget.Get()->GetActorLocation());
		}
	}

	// --- Slowdown (friction) ---
	float SlowdownAngle = 1.f;
	if (Settings->FrictionByAngle)
	{
		// Usually driven by yaw error in degrees; you can combine yaw/pitch if desired.
		SlowdownAngle = Settings->FrictionByAngle->GetFloatValue(AbsYawErr);
	}

	float SlowdownDist = 1.f;
	if (Settings->FrictionByDistance)
	{
		SlowdownDist = Settings->FrictionByDistance->GetFloatValue(DistanceCm);
	}

	const float Slowdown = FMath::Clamp(SlowdownAngle * SlowdownDist, 0.f, 1.f);

	// Apply slowdown multiplicatively
	FVector2D Adjusted = RawLookInput * Slowdown;

	// --- Magnetism (adhesion) ---
	if (Settings->MagnetismByAngle)
	{
		// Blend speed is a rate [1/sec] controlling how quickly we move towards the target direction.
		const float Blend = FMath::Clamp(
			Settings->MagnetismByAngle->GetFloatValue(AbsYawErr) * Settings->MagnetismBlendSpeed * DeltaSeconds,
			0.f, 1.f);

		const float YawNudgeDeg   = YawErrDeg   * Blend;
		const float PitchNudgeDeg = PitchErrDeg * Blend;

		// Convert degrees to your input axis units (helpers should exist in your component).
		Adjusted.X += DegToInputYaw(YawNudgeDeg);
		Adjusted.Y += DegToInputPitch(PitchNudgeDeg);
	}

	// Debug readout
	const bool bDebug = bDrawDebug || (Settings && Settings->bDebugDraw);
	if (bDebug)
	{
		if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			if (const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
			{
				if (const APlayerCameraManager* PCM = PC->PlayerCameraManager)
				{
					const FVector CamLoc    = PCM->GetCameraLocation();
					const FVector TargetLoc = (CurrentTarget.IsValid() ? CurrentTarget.Get()->GetActorLocation() : CamLoc);
					DrawDebugLine(GetWorld(), CamLoc, TargetLoc, FColor::Yellow, false, 0.05f, 0, 0.5f);
					DrawDebugString(GetWorld(), TargetLoc + FVector(0,0,50.f),
						FString::Printf(TEXT("YawErr: %.1f  Slowdown: %.2f"), YawErrDeg, Slowdown),
						nullptr, FColor::White, 0.05f, false);
				}
			}
		}
	}

	return Adjusted;
}
