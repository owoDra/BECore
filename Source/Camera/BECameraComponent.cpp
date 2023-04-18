// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BECameraComponent.h"

#include "BECameraMode.h"

#include "Camera/CameraTypes.h"
#include "Containers/EnumAsByte.h"
#include "Containers/UnrealString.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Math/Color.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathSSE.h"
#include "Math/Vector.h"
#include "Misc/AssertionMacros.h"
#include "Templates/Casts.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECameraComponent)


UBECameraComponent::UBECameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CameraModeStack = nullptr;
}

void UBECameraComponent::OnRegister()
{
	Super::OnRegister();

	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UBECameraModeStack>(this);
		check(CameraModeStack);
	}
}

void UBECameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	check(CameraModeStack);

	UpdateCameraModes();
	UpdateZoomAmount(DeltaTime);

	FBECameraModeView CameraModeView;
	CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

	// Keep player controller in sync with the latest view.
	if (APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
	{
		if (APlayerController* PC = TargetPawn->GetController<APlayerController>())
		{
			const FRotator SuppressedRotation = UpdateRecoilOffset(DeltaTime, CameraModeView.ControlRotation - PrevControlRotation);

			PrevControlRotation = CameraModeView.ControlRotation;
			CameraModeView.ControlRotation += SuppressedRotation;
			CameraModeView.Rotation += RecoilOffset;

			PC->SetControlRotation(CameraModeView.ControlRotation);
		}
	}

	// Zoom を適応
	CameraModeView.FieldOfView *= (1.0/CurrentZoomAmount);

	// Keep camera component in sync with the latest view.
	SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
	FieldOfView = CameraModeView.FieldOfView;

	// Fill in desired view.
	DesiredView.Location = CameraModeView.Location;
	DesiredView.Rotation = CameraModeView.Rotation;
	DesiredView.FOV = CameraModeView.FieldOfView;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;

	// See if the CameraActor wants to override the PostProcess settings used.
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}
}

void UBECameraComponent::UpdateCameraModes()
{
	check(CameraModeStack);

	if (CameraModeStack->IsStackActivate())
	{
		if (DetermineCameraModeDelegate.IsBound())
		{
			if (const TSubclassOf<UBECameraMode> CameraMode = DetermineCameraModeDelegate.Execute())
			{
				CameraModeStack->PushCameraMode(CameraMode);

				float Weight;
				FGameplayTag CameraTag;
				GetBlendInfo(Weight, CameraTag);
				OnCameraModeChanged.Broadcast(Weight, CameraTag);
			}
		}
	}
}

FRotator UBECameraComponent::UpdateRecoilOffset(float DeltaTime, FRotator DeltaRotation)
{
	FRotator SuppressedAmount = FRotator::ZeroRotator;

	if (CanRecoilRecovery)
	{
		RecoilOffset = FMath::RInterpTo(RecoilOffset, FRotator(0.0), DeltaTime, 4.0);
		TargetRecoilOffset = FMath::RInterpTo(TargetRecoilOffset, FRotator(0.0), DeltaTime, 4.0);

		if (RecoilOffset.IsNearlyZero() && TargetRecoilOffset.IsNearlyZero())
		{
			CanRecoilRecovery = false;
		}
	}
	else
	{
		RecoilOffset = FMath::RInterpTo(RecoilOffset, TargetRecoilOffset, DeltaTime, 16.0);

		if (!RecoilOffset.IsNearlyZero())
		{
			// 縦リコイルを相殺
			float DeltaVert = DeltaRotation.Pitch;
			float TargetVert = RecoilOffset.Pitch;

			if ((TargetVert > 0) && (DeltaVert < 0))
			{
				TargetVert = FMath::Max(0, TargetVert + DeltaVert);
			}
			else if ((TargetVert < 0) && (DeltaVert > 0))
			{
				TargetVert = FMath::Min(0, TargetVert + DeltaVert);
			}

			// 横リコイルを相殺
			float DeltaHori = DeltaRotation.Yaw;
			float TargetHori = RecoilOffset.Yaw;

			if ((TargetHori > 0) && (DeltaHori < 0))
			{
				TargetHori = FMath::Max(0, TargetHori + DeltaHori);
			}
			else if ((TargetHori < 0) && (DeltaHori > 0))
			{
				TargetHori = FMath::Min(0, TargetHori + DeltaHori);
			}

			const FRotator ModifiedOffset = FRotator(TargetVert, TargetHori, 0.0);

			SuppressedAmount = RecoilOffset - ModifiedOffset;

			TargetRecoilOffset -= SuppressedAmount;

			RecoilOffset = ModifiedOffset;
		}
	}

	return SuppressedAmount;
}

void UBECameraComponent::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("BECameraComponent: %s"), *GetNameSafe(GetTargetActor())));

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Location: %s"), *GetComponentLocation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Rotation: %s"), *GetComponentRotation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   FOV: %f"), FieldOfView));

	check(CameraModeStack);
	CameraModeStack->DrawDebug(Canvas);
}

void UBECameraComponent::SetZoom(float ZoomAmount, float InterpSpeed)
{
	isZooming = true;
	TargetZoomAmount = ZoomAmount;
	ZoomInterpSpeed = InterpSpeed;
}

void UBECameraComponent::UpdateZoomAmount(float DeltaTime)
{
	if (!isZooming)
	{
		return;
	}

	CurrentZoomAmount = FMath::FInterpTo(CurrentZoomAmount, TargetZoomAmount, DeltaTime, ZoomInterpSpeed);

	if (CurrentZoomAmount == TargetZoomAmount)
	{
		isZooming = false;
	}
}

void UBECameraComponent::AddRecoilOffset(FVector2D Offset, float RecoveryDelay)
{
	TargetRecoilOffset.Yaw += Offset.X;
	TargetRecoilOffset.Pitch += Offset.Y;

	CanRecoilRecovery = false;

	if (GetWorld()->GetTimerManager().TimerExists(TimerRecoilRecovery) ||
		GetWorld()->GetTimerManager().IsTimerActive(TimerRecoilRecovery))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerRecoilRecovery);
	}

	GetWorld()->GetTimerManager().SetTimer(TimerRecoilRecovery, this, &ThisClass::HandleRecoilRecovery, 1.0, false, RecoveryDelay);
}

void UBECameraComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(/*out*/ OutWeightOfTopLayer, /*out*/ OutTagOfTopLayer);
}

void UBECameraComponent::HandleRecoilRecovery()
{
	CanRecoilRecovery = true;
}

