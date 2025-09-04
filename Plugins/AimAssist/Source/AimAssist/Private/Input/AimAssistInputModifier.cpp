#include "Input/AimAssistInputModifier.h"
#include "Input/AimAssistDataAsset.h"

#include "EnhancedPlayerInput.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Actor.h"

namespace
{
	// Fallback tunables if you don't expose them in the DataAsset yet.
	constexpr float kMinStickForAssist = 0.05f;     // deadzone before we do anything
	constexpr float kMagnetismBlendSpeed = 8.0f;    // how fast we blend toward target dir (per second)
}

float UAimAssistInputModifier::AngleBetweenViewAndPoint(const APlayerController* PC, const FVector& WorldPoint)
{
	if (!PC || !PC->PlayerCameraManager)
		return 180.f;

	const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FVector CamDir = PC->PlayerCameraManager->GetActorForwardVector().GetSafeNormal();

	const FVector ToTarget = (WorldPoint - CamLoc).GetSafeNormal();
	const float CosAngle = FVector::DotProduct(CamDir, ToTarget);
	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAngle, -1.f, 1.f)));
}

bool UAimAssistInputModifier::ShouldApply(const UEnhancedPlayerInput* PlayerInput) const
{
	if (!Settings || !PlayerInput)
		return false;

	// Device gating: if you need strict device checks, handle it via separate IMCs or your own device tracking.
	if (Settings->bOnlyGamepad)
	{
		// Minimal, non-invasive gate: assume this modifier is bound only in the Gamepad IMC.
		// Hook up your own device detection if you want a hard check here.
	}

	return true;
}

bool UAimAssistInputModifier::FindBestTarget(const APlayerController* PC, FVector& OutWorld, float& OutAngle) const
{
	OutAngle = 999.f;
	if (!PC || !Settings || !PC->PlayerCameraManager)
		return false;

	UWorld* World = PC->GetWorld();
	if (!World)
		return false;

	const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();

	// Overlap a small sphere to gather nearby candidates (you can replace with a richer query).
	TArray<FOverlapResult> Hits;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AimAssistOverlap), /*bTraceComplex=*/false);
	FCollisionObjectQueryParams ObjParams;

	if (Settings->TargetObjectChannels.Num() == 0)
	{
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);
	}
	else
	{
		for (const TEnumAsByte<ECollisionChannel> Chan : Settings->TargetObjectChannels)
		{
			ObjParams.AddObjectTypesToQuery(Chan);
		}
	}

	// Simple heuristic for scan radius (or expose separately in Settings if you prefer).
	const float Radius = FMath::Max(32.f, Settings->MaxTargetDistance * 0.1f);

	World->OverlapMultiByObjectType(
		Hits,
		CamLoc,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	bool bFound = false;
	for (const FOverlapResult& Result : Hits)
	{
		const AActor* Actor = Result.GetActor();
		if (!Actor)
			continue;

		// Skip self pawn
		if (const APawn* MyPawn = PC->GetPawn())
		{
			if (Actor == MyPawn)
				continue;
		}

		const FVector Loc = Actor->GetActorLocation();
		const float Dist = FVector::Dist(CamLoc, Loc);
		if (Dist > Settings->MaxTargetDistance)
			continue;

		const float Angle = AngleBetweenViewAndPoint(PC, Loc);
		if (Angle <= Settings->AcquisitionFOVDeg && Angle < OutAngle)
		{
			OutAngle = Angle;
			OutWorld = Loc;
			bFound = true;
		}
	}

#if !(UE_BUILD_SHIPPING)
	// If you want a toggle here, add a bool bDebugDraw in the DataAsset and gate this on it.
	if (bFound)
	{
		DrawDebugSphere(World, OutWorld, 16.f, 12, FColor::Green, false, 0.05f);
		DrawDebugLine(World, CamLoc, OutWorld, FColor::Green, false, 0.05f, 0, 1.5f);
	}
#endif

	return bFound;
}

FInputActionValue UAimAssistInputModifier::ModifyRaw_Implementation(
	const UEnhancedPlayerInput* PlayerInput,
	FInputActionValue CurrentValue,
	float DeltaTime)
{
	if (!Settings)
		return CurrentValue;

	// Expect a 2D Look input (X=yaw, Y=pitch).
	const FVector2D In = CurrentValue.Get<FVector2D>();

	if (!ShouldApply(PlayerInput))
		return CurrentValue;

	// Ignore small stick magnitudes to prevent jitter.
	if (In.Size() < kMinStickForAssist)
		return CurrentValue;

	const APlayerController* PC = Cast<APlayerController>(PlayerInput ? PlayerInput->GetOuter() : nullptr);
	if (!PC)
		return CurrentValue;

	// Try to acquire a target.
	FVector TargetWorld = FVector::ZeroVector;
	float AngleDeg = 0.f;
	const bool bHasTarget = FindBestTarget(PC, TargetWorld, AngleDeg);

	FVector2D Out = In;

	// ---------------------
	// SLOWDOWN / FRICTION
	// ---------------------
	if (bHasTarget && Settings->FrictionByAngle)
	{
		float SlowFactor = FMath::Clamp(Settings->FrictionByAngle->GetFloatValue(AngleDeg), 0.f, 1.f);

		if (Settings->FrictionByDistance && PC->PlayerCameraManager)
		{
			const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
			const float Dist = FVector::Dist(CamLoc, TargetWorld);
			SlowFactor *= FMath::Clamp(Settings->FrictionByDistance->GetFloatValue(Dist), 0.f, 1.f);
		}

		// Clamp with optional lower bound if you expose it (e.g., Settings->MinSlowdown). Otherwise just clamp [0..1].
		Out *= SlowFactor;
	}

	// -----------
	// MAGNETISM
	// -----------
	if (bHasTarget && Settings->MagnetismByAngle && PC->PlayerCameraManager)
	{
		const FVector CamLoc   = PC->PlayerCameraManager->GetCameraLocation();
		const FVector CamRight = PC->PlayerCameraManager->GetActorRightVector();
		const FVector CamUp    = PC->PlayerCameraManager->GetActorUpVector();
		const FVector CamFwd   = PC->PlayerCameraManager->GetActorForwardVector();

		const FVector ToTarget = (TargetWorld - CamLoc).GetSafeNormal();

		// Project world direction into camera (screen) space
		const float RightDot = FVector::DotProduct(ToTarget, CamRight);
		const float UpDot    = FVector::DotProduct(ToTarget, CamUp);
		/* const float FwdDot = FVector::DotProduct(ToTarget, CamFwd); */ // reserved if you want weighting by depth

		const FVector2D TargetDirScreen(RightDot, UpDot);
		const FVector2D TargetDirN = TargetDirScreen.GetSafeNormal();
		const FVector2D InN = In.GetSafeNormal();

		const float Mag = FMath::Clamp(Settings->MagnetismByAngle->GetFloatValue(AngleDeg), 0.f, 1.f);

		// Blend only direction; keep magnitude (after slowdown).
		const float Magnitude = Out.Size();
		const float Alpha = FMath::Clamp(Mag * kMagnetismBlendSpeed * DeltaTime, 0.f, 1.f);
		const FVector2D BlendedDir = (InN * (1.f - Alpha)) + (TargetDirN * Alpha);
		Out = BlendedDir.GetSafeNormal() * Magnitude;
	}

	return FInputActionValue(Out);
}
