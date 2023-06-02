// Copyright Eigi Chin

#include "BECameraMode.h"

#include "Camera/BECameraComponent.h"
#include "GameplayTag/BETags_Camera.h"

#include "Components/CapsuleComponent.h"
#include "Containers/UnrealString.h"
#include "Engine/Canvas.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Math/Color.h"
#include "Misc/AssertionMacros.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealNames.h"

#define BE_CAMERA_DEFAULT_FOV		(90.0f)
#define BE_CAMERA_DEFAULT_PITCH_MIN	(-89.0f)
#define BE_CAMERA_DEFAULT_PITCH_MAX	(89.0f)

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECameraMode)


//////////////////////////////////////////////////////////////////////////
// FBECameraModeView
//////////////////////////////////////////////////////////////////////////

FBECameraModeView::FBECameraModeView()
	: Location(ForceInit)
	, Rotation(ForceInit)
	, ControlRotation(ForceInit)
	, FieldOfView(BE_CAMERA_DEFAULT_FOV)
{
}

void FBECameraModeView::Blend(const FBECameraModeView& Other, float OtherWeight)
{
	if (OtherWeight <= 0.0f)
	{
		return;
	}
	else if (OtherWeight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, OtherWeight);

	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (OtherWeight * DeltaRotation);

	const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
	ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}


//////////////////////////////////////////////////////////////////////////
// UBECameraMode
//////////////////////////////////////////////////////////////////////////

UBECameraMode::UBECameraMode()
{
	FieldOfView = 90.0;
	ViewPitchMin = BE_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = BE_CAMERA_DEFAULT_PITCH_MAX;

	BlendTime = 0.5f;
	BlendFunction = EBECameraModeBlendFunction::EaseOut;
	BlendExponent = 4.0f;
	BlendAlpha = 1.0f;
	BlendWeight = 1.0f;

	ActivationState = EBECameraModeActivationState::Deactevated;
}


void UBECameraMode::SetActivationState(EBECameraModeActivationState NewActivationState)
{
	if (ActivationState == NewActivationState)
	{
		return;
	}

	ActivationState = NewActivationState;

	if (ActivationState == EBECameraModeActivationState::PreActivate)
	{
		PreActivateMode();
	}
	else if (ActivationState == EBECameraModeActivationState::Activated)
	{
		PostActivateMode();
	}
	else if (ActivationState == EBECameraModeActivationState::PreDeactevate)
	{
		PreDeactivateMode();
	}
	else if (ActivationState == EBECameraModeActivationState::Deactevated)
	{
		PostDeactivateMode();
	}
}


FVector UBECameraMode::GetPivotLocation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// Height adjustments for characters to account for crouching.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			check(TargetCharacterCDO);

			const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
			check(CapsuleComp);

			const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
			check(CapsuleCompCDO);

			const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
			const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
			const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;

			return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
		}

		return TargetPawn->GetPawnViewLocation();
	}

	return TargetActor->GetActorLocation();
}

FRotator UBECameraMode::GetPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetViewRotation();
	}

	return TargetActor->GetActorRotation();
}

void UBECameraMode::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void UBECameraMode::UpdateBlending(float DeltaTime)
{
	if (BlendTime > 0.0f)
	{
		BlendAlpha += (DeltaTime / BlendTime);
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

	switch (BlendFunction)
	{
	case EBECameraModeBlendFunction::Linear:
		BlendWeight = BlendAlpha;
		break;

	case EBECameraModeBlendFunction::EaseIn:
		BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EBECameraModeBlendFunction::EaseOut:
		BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EBECameraModeBlendFunction::EaseInOut:
		BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	default:
		checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void UBECameraMode::UpdateCameraMode(float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void UBECameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
	const float InvExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

	switch (BlendFunction)
	{
	case EBECameraModeBlendFunction::Linear:
		BlendAlpha = BlendWeight;
		break;

	case EBECameraModeBlendFunction::EaseIn:
		BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EBECameraModeBlendFunction::EaseOut:
		BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EBECameraModeBlendFunction::EaseInOut:
		BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	default:
		checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}


UBECameraComponent* UBECameraMode::GetBECameraComponent() const
{
	return CastChecked<UBECameraComponent>(GetOuter());
}

UWorld* UBECameraMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UBECameraMode::GetTargetActor() const
{
	const UBECameraComponent* BECameraComponent = GetBECameraComponent();

	return BECameraComponent->GetOwner();
}


//////////////////////////////////////////////////////////////////////////
// UBECameraModeStack
//////////////////////////////////////////////////////////////////////////

UBECameraModeStack::UBECameraModeStack()
{
}


UBECameraMode* UBECameraModeStack::GetCameraModeInstance(TSubclassOf<UBECameraMode> CameraModeClass)
{
	check(CameraModeClass);

	// 既にインスタンスが作成されているか確認
	for (UBECameraMode* CameraMode : CameraModeInstances)
	{
		if ((CameraMode != nullptr) && (CameraMode->GetClass() == CameraModeClass))
		{
			return CameraMode;
		}
	}

	// 既にインスタンスが作成されていない場合は作成する
	UBECameraMode* NewCameraMode = NewObject<UBECameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
	check(NewCameraMode);

	// インスタンスを配列に追加し後から参照できるようにする
	CameraModeInstances.Add(NewCameraMode);

	return NewCameraMode;
}


void UBECameraModeStack::UpdateStack(float DeltaTime)
{
	const int32 StackSize = CameraModeStack.Num();

	// Stack が 0 以下(つまり空)の場合はスキップ
	if (StackSize <= 0)
	{
		return;
	}

	int32 RemoveCount = 0;
	int32 RemoveIndex = INDEX_NONE;

	// Stack 内の CameraMode をすべて更新する
	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		UBECameraMode* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		CameraMode->UpdateCameraMode(DeltaTime);

		// CameraMode の BlendWeight が 1.0 以上(つまり Blend が完了済み)かどうか
		if (CameraMode->GetBlendWeight() >= 1.0f)
		{
			// Blend が完了している CameraMode より前に追加された最初の項目の Index とそこから何個の項目を削除するか記録
			RemoveIndex = (StackIndex + 1);
			RemoveCount = (StackSize - RemoveIndex);

			// Index 0 の Blend が完了したということは完全に Active になったということなので
			// ActivationState を Activated に設定
			if (StackIndex == 0)
			{
				CameraMode->SetActivationState(EBECameraModeActivationState::Activated);
			}
			break;
		}
	}

	// Blend が完了し除外しても問題ない項目があるか
	if (RemoveCount > 0)
	{
		// 記録した項目を削除する
		for (int32 StackIndex = RemoveIndex; StackIndex < StackSize; ++StackIndex)
		{
			UBECameraMode* CameraMode = CameraModeStack[StackIndex];
			check(CameraMode);

			// ActivationState を Deactevated 設定する
			CameraMode->SetActivationState(EBECameraModeActivationState::Deactevated);
		}

		CameraModeStack.RemoveAt(RemoveIndex, RemoveCount);
	}
}

void UBECameraModeStack::BlendStack(FBECameraModeView& OutCameraModeView) const
{
	const int32 LastIndex = CameraModeStack.Num() - 1;

	// Stack が空の場合はスキップ
	if (LastIndex < 0)
	{
		return;
	}

	// Stack の一番最後の CameraMode から順番に Blend する
	const UBECameraMode* CameraMode = CameraModeStack[LastIndex];
	check(CameraMode);

	OutCameraModeView = CameraMode->GetCameraModeView();

	for (int32 StackIndex = (LastIndex - 1); StackIndex >= 0; --StackIndex)
	{
		CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
	}
}

void UBECameraModeStack::PushCameraMode(TSubclassOf<UBECameraMode> CameraModeClass)
{
	// 新しく適応する CameraMode が有効かどうか
	if (!CameraModeClass)
	{
		return;
	}

	// CameraMode からインスタンスを作成、またはキャッシュから取得
	UBECameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);
	check(CameraMode);

	int32 StackSize = CameraModeStack.Num();

	// 追加しようとしている CameraMode が既に Stack の一番上(つまり有効化済み、または有効か途中)かどうか
	if ((StackSize > 0) && (CameraModeStack[0] == CameraMode))
	{
		return;
	}

	// Stack に既にあるかどうかを確認し、Stack 内の CameraMode の BlendWeight から
	// 新しく追加する CameraMode の BlendWeight を決める
	int32 ExistingStackIndex		= INDEX_NONE;
	float ExistingStackContribution = 1.0f;
	bool bCameraModeExisted			= false;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		const bool bSameCameraMode = (CameraModeStack[StackIndex] == CameraMode);

		// 新しく追加する CameraMode が存在するかわかっていない間だけ ExistingStackContribution の計算を続ける
		if (bCameraModeExisted == false)
		{
			if (bSameCameraMode)
			{
				ExistingStackIndex = StackIndex;
				ExistingStackContribution *= CameraMode->GetBlendWeight();

				// CameraMode が存在しているとわかったのでフラグを立てる
				bCameraModeExisted = true;
			}
			else
			{
				ExistingStackContribution *= (1.0f - CameraModeStack[StackIndex]->GetBlendWeight());
			}
		}

		// 新しく追加する CameraMode と同じではないときにだけ、その CameraMode の ActivationState を PreDeactevate にする
		if (bSameCameraMode == false)
		{
			CameraModeStack[StackIndex]->SetActivationState(EBECameraModeActivationState::PreDeactevate);
		}
	}

	// Stack に既にないなら全体の BlendWeight に影響しないので 0.0 に設定
	if (ExistingStackIndex == INDEX_NONE)
	{
		ExistingStackContribution = 0.0f;
	}

	// Stack に既にあるなら削除する
	else
	{
		CameraModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	}

	// 新しく追加する CameraMode の BlendWeight を決める
	const bool bShouldBlend = ((CameraMode->GetBlendTime() > 0.0f) && (StackSize > 0));
	const float BlendWeight = (bShouldBlend ? ExistingStackContribution : 1.0f);

	CameraMode->SetBlendWeight(BlendWeight);

	// Stack の最初に挿入
	CameraModeStack.Insert(CameraMode, 0);

	// Stack の最後の項目の BlendWeight を 1.0 (100%)にする
	CameraModeStack.Last()->SetBlendWeight(1.0f);

	// CameraMode の ActivationState を PreActivate に設定
	CameraMode->SetActivationState(EBECameraModeActivationState::PreActivate);
}

void UBECameraModeStack::EvaluateStack(float DeltaTime, FBECameraModeView& OutCameraModeView)
{
	UpdateStack(DeltaTime);
	BlendStack(OutCameraModeView);
}

void UBECameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	if (CameraModeStack.IsEmpty())
	{
		OutWeightOfTopLayer = 1.0f;
		OutTagOfTopLayer = FGameplayTag();
	}
	else
	{
		UBECameraMode* TopEntry = CameraModeStack[0];
		check(TopEntry);
		OutWeightOfTopLayer = TopEntry->GetBlendWeight();
		OutTagOfTopLayer = TopEntry->GetCameraTypeTag();
	}
}
