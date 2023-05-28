// Copyright Eigi Chin

#include "BECameraComponent.h"

#include "Camera/CameraTypes.h"
#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathSSE.h"
#include "Math/Vector.h"
#include "Misc/AssertionMacros.h"
#include "Templates/Casts.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECameraComponent)


UBECameraComponent::UBECameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
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
	ComputeCameraView(DeltaTime, DesiredView);
}


void UBECameraComponent::ComputeCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	APlayerController* PlayerController = GetOwnerController();
	check(PlayerController);
	
	FBECameraModeView CameraModeView;
	CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

	ControlRotationDelta = (CameraModeView.ControlRotation - PreviousControlRotation);
	
	ComputeRecoilAmount(DeltaTime, CameraModeView);
	ComputeZoomAmount(DeltaTime, CameraModeView);
	
	SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
	PreviousControlRotation = CameraModeView.ControlRotation;
	PlayerController->SetControlRotation(CameraModeView.ControlRotation);
	FieldOfView = CameraModeView.FieldOfView;

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
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}
}

void UBECameraComponent::ComputeRecoilAmount(float DeltaTime, FBECameraModeView& CameraModeView)
{
	// Recoil の適応
	if (RecoilState == EBECameraRecoilState::RecoilUp)
	{
		// このフレームにおける ControlRotation のデルタ
		const FRotator ControlDelta = ControlRotationDelta;
		const bool bNegativeDeltaY = (ControlDelta.Pitch < 0.0);
		const bool bNegativeDeltaX = (ControlDelta.Yaw < 0.0);

		// このフレームに適応する Recoil の追加回転量
		const FRotator RecoilAmount = RecoilAmountToAdd * DeltaTime * 16;
		const bool bNegativeRecoilY = (RecoilAmount.Pitch < 0.0);
		const bool bNegativeRecoilX = (RecoilAmount.Yaw < 0.0);

		RecoilAmountToAdd -= RecoilAmount;

		// リコイル抑制可能なら適応
		float SuppressedRecoilY, SuppressedRecoilX = 0.0;

		if (bNegativeDeltaY != bNegativeRecoilY)
		{
			SuppressedRecoilY = (RecoilAmount.Pitch + ControlDelta.Pitch);
			SuppressedRecoilY = bNegativeRecoilY ? FMath::Min(SuppressedRecoilY, 0.0) : FMath::Max(SuppressedRecoilY, 0.0);
		}
		if (bNegativeDeltaX != bNegativeRecoilX)
		{
			SuppressedRecoilX = (RecoilAmount.Yaw + ControlDelta.Yaw);
			SuppressedRecoilX = bNegativeRecoilX ? FMath::Min(SuppressedRecoilX, 0.0) : FMath::Max(SuppressedRecoilX, 0.0);
		}

		const FRotator SuppressedRecoil = FRotator(SuppressedRecoilY, SuppressedRecoilX, 0.0);

		// リコイルを適応
		CameraModeView.ControlRotation += SuppressedRecoil;
		CurrentRecoilAmount += SuppressedRecoil;

		// リコイルの適応が終わったか判定し、終わっていたら RecoilState を None にする。
		if (RecoilAmountToAdd.IsNearlyZero())
		{
			RecoilAmountToAdd = FRotator::ZeroRotator;
			RecoilState = EBECameraRecoilState::None;
		}
	}

	// Recovery の適応
	else if (RecoilState == EBECameraRecoilState::Recovery)
	{
		// このフレームに適応する Recoil の復元回転量
		const FRotator RecoveryAmount = CurrentRecoilAmount * DeltaTime * 4;

		// Recovery を適応
		CameraModeView.ControlRotation -= RecoveryAmount;
		CurrentRecoilAmount -= RecoveryAmount;

		// Recovery の適応が終わったか判定し、終わっていたら RecoilState を None にする。
		if (CurrentRecoilAmount.IsNearlyZero())
		{
			CurrentRecoilAmount = FRotator::ZeroRotator;
			RecoilState = EBECameraRecoilState::None;
		}
	}
}

void UBECameraComponent::ComputeZoomAmount(float DeltaTime, FBECameraModeView& CameraModeView)
{
	if (isZooming)
	{
		CurrentZoomAmount = FMath::FInterpTo(CurrentZoomAmount, TargetZoomAmount, DeltaTime, ZoomInterpSpeed);

		if (CurrentZoomAmount == TargetZoomAmount)
		{
			isZooming = false;
		}
	}

	CameraModeView.FieldOfView *= (1.0 / CurrentZoomAmount);
}


void UBECameraComponent::AddRecoilOffset(FVector2D Offset, float RecoveryDelay)
{
	RecoilAmountToAdd.Yaw += Offset.X;
	RecoilAmountToAdd.Pitch += Offset.Y;

	RecoilState = EBECameraRecoilState::RecoilUp;

	if (GetWorld()->GetTimerManager().TimerExists(TimerRecoilRecovery) ||
		GetWorld()->GetTimerManager().IsTimerActive(TimerRecoilRecovery))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerRecoilRecovery);
	}
	GetWorld()->GetTimerManager().SetTimer(TimerRecoilRecovery, this, &ThisClass::HandleRecoilRecovery, 1.0, false, RecoveryDelay);
}

void UBECameraComponent::HandleRecoilRecovery()
{
	RecoilAmountToAdd = FRotator::ZeroRotator;
	RecoilState = EBECameraRecoilState::Recovery;
}


void UBECameraComponent::SetZoom(float ZoomAmount, float InterpSpeed)
{
	isZooming = true;
	TargetZoomAmount = ZoomAmount;
	ZoomInterpSpeed = InterpSpeed;
}


APlayerController* UBECameraComponent::GetOwnerController() const
{
	if (APawn* TargetPawn = Cast<APawn>(GetOwner()))
	{
		return TargetPawn->GetController<APlayerController>();
	}

	return nullptr;
}

UBECameraComponent* UBECameraComponent::FindCameraComponent(const APawn* Pawn)
{
	return (Pawn ? Pawn->FindComponentByClass<UBECameraComponent>() : nullptr);
}

void UBECameraComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(OutWeightOfTopLayer, OutTagOfTopLayer);
}

void UBECameraComponent::PushCameraMode(TSubclassOf<UBECameraMode> CameraModeClass)
{
	check(CameraModeStack);
	CameraModeStack->PushCameraMode(CameraModeClass);
}
