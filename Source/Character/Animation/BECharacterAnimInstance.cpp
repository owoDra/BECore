// Copyright owoDra

#include "BECharacterAnimInstance.h"

#include "Character/Animation/BECharacterAnimInstanceProxy.h"
#include "Character/Animation/BECharacterAnimData.h"
#include "Character/Animation/BECharacterAnimCurveNameStatics.h"
#include "Character/Component/BECharacterMovementComponent.h"
#include "Character/Movement/BEMovementMathLibrary.h"
#include "Character/BECharacter.h"
#include "GameplayTag/BETags_Status.h"
#include "Development/BEDeveloperStatGroup.h"

#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterAnimInstance)


UBECharacterAnimInstance::UBECharacterAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
}

void UBECharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ABECharacter>(GetOwningActor());

#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld() && !IsValid(Character))
	{
		// エディターでの表示用に BECharacter の CDO を使用する

		Character = GetMutableDefault<ABECharacter>();
	}
#endif

	CharacterMovement = Character->GetBECharacterMovement();
}

void UBECharacterAnimInstance::NativeBeginPlay()
{
	ensureMsgf(IsValid(AnimData)			, TEXT("UBECharacterAnimInstance::NativeBeginPlay: Invalid AnimData"));
	ensureMsgf(IsValid(Character)			, TEXT("UBECharacterAnimInstance::NativeBeginPlay: Invalid Character"));
	ensureMsgf(IsValid(CharacterMovement)	, TEXT("UBECharacterAnimInstance::NativeBeginPlay: Invalid CharacterMovement"));
}

void UBECharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UBECharacterAnimInstance::NativeUpdateAnimation()"), STAT_UBECharacterAnimInstance_NativeUpdateAnimation, STATGROUP_BEMovement);

	Super::NativeUpdateAnimation(DeltaTime);

	if (!IsValid(AnimData) || !IsValid(Character) || !IsValid(CharacterMovement))
	{
		return;
	}

	if (GetSkelMeshComponent()->IsUsingAbsoluteRotation())
	{
		const auto& ActorTransform{ Character->GetActorTransform() };

		// メッシュの回転をキャラクターの回転と手動で同期

		GetSkelMeshComponent()->MoveComponent(FVector::ZeroVector, ActorTransform.GetRotation() * Character->GetBaseRotationOffset(), false);

		// Re-cache proxy transforms to match the modified mesh transform.

		const auto& Proxy{ GetProxyOnGameThread<FAnimInstanceProxy>() };

		const_cast<FTransform&>(Proxy.GetComponentTransform())			= GetSkelMeshComponent()->GetComponentTransform();
		const_cast<FTransform&>(Proxy.GetComponentRelativeTransform())	= GetSkelMeshComponent()->GetRelativeTransform();
		const_cast<FTransform&>(Proxy.GetActorTransform())				= ActorTransform;
	}

	LocomotionMode		= CharacterMovement->GetLocomotionMode();
	RotationMode		= CharacterMovement->GetRotationMode();
	Stance				= CharacterMovement->GetStance();
	Gait				= CharacterMovement->GetGait();
	LocomotionAction	= CharacterMovement->GetLocomotionAction();

	UpdateMovementBaseOnGameThread();
	UpdateViewOnGameThread();
	UpdateLocomotionOnGameThread();
	UpdateGroundedOnGameThread();
	UpdateInAirOnGameThread();

	UpdateFeetOnGameThread();
}

void UBECharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UBECharacterAnimInstance::NativeThreadSafeUpdateAnimation()"), STAT_UBECharacterAnimInstance_NativeThreadSafeUpdateAnimation, STATGROUP_BEMovement);

	Super::NativeThreadSafeUpdateAnimation(DeltaTime);

	if (!IsValid(AnimData) || !IsValid(Character) || !IsValid(CharacterMovement))
	{
		return;
	}

	UpdateLayering();
	UpdatePose();

	UpdateView(DeltaTime);
	UpdateGrounded(DeltaTime);
	UpdateInAir(DeltaTime);

	UpdateFeet(DeltaTime);

	UpdateTransitions();
	UpdateRotateInPlace(DeltaTime);
}

void UBECharacterAnimInstance::NativePostEvaluateAnimation()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UBECharacterAnimInstance::NativePostEvaluateAnimation()"), STAT_UBECharacterAnimInstance_NativePostEvaluateAnimation, STATGROUP_BEMovement)

	Super::NativePostEvaluateAnimation();

	if (!IsValid(AnimData) || !IsValid(Character) || !IsValid(CharacterMovement))
	{
		return;
	}

	PlayQueuedDynamicTransitionAnimation();

	bPendingUpdate = false;
}

FAnimInstanceProxy* UBECharacterAnimInstance::CreateAnimInstanceProxy()
{
	return new FBECharacterAnimInstanceProxy(this);
}


#pragma region Core

FControlRigInput UBECharacterAnimInstance::GetControlRigInput() const
{
	return {
		(!IsValid(AnimData) || AnimData->General.bUseHandIkBones),
		(!IsValid(AnimData) || AnimData->General.bUseFootIkBones),
		GroundedState.VelocityBlend.ForwardAmount,
		GroundedState.VelocityBlend.BackwardAmount,
		ViewState.SpineRotation.YawAngle,
		FeetState.Left.IkRotation,
		FeetState.Left.IkLocation,
		FeetState.Left.IkAmount,
		FeetState.Right.IkRotation,
		FeetState.Right.IkLocation,
		FeetState.Right.IkAmount,
		FeetState.MinMaxPelvisOffsetZ,
	};
}

void UBECharacterAnimInstance::MarkTeleported()
{
	TeleportedTime = GetWorld()->GetTimeSeconds();
}

void UBECharacterAnimInstance::UpdateMovementBaseOnGameThread()
{
	const auto& BasedMovement{ Character->GetBasedMovement() };

	if (BasedMovement.MovementBase != MovementBase.Primitive || BasedMovement.BoneName != MovementBase.BoneName)
	{
		MovementBase.Primitive = BasedMovement.MovementBase;
		MovementBase.BoneName = BasedMovement.BoneName;
		MovementBase.bBaseChanged = true;
	}
	else
	{
		MovementBase.bBaseChanged = false;
	}

	MovementBase.bHasRelativeLocation = BasedMovement.HasRelativeLocation();
	MovementBase.bHasRelativeRotation = MovementBase.bHasRelativeLocation && BasedMovement.bRelativeRotation;

	const auto PreviousRotation{ MovementBase.Rotation };

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName, MovementBase.Location, MovementBase.Rotation);

	MovementBase.DeltaRotation = (MovementBase.bHasRelativeLocation && !MovementBase.bBaseChanged)
								? (MovementBase.Rotation * PreviousRotation.Inverse()).Rotator()
								: FRotator::ZeroRotator;
}

void UBECharacterAnimInstance::UpdateLayering()
{
	const auto& Curves{ GetProxyOnAnyThread<FBECharacterAnimInstanceProxy>().GetAnimationCurves(EAnimCurveType::AttributeCurve) };

	// AnimCurve の値を取得するラムダ関数

	static const auto GetCurveValue = [](const TMap<FName, float>& Curves, const FName& CurveName) -> float
										{
											const float* Value = Curves.Find(CurveName);

											return (Value != nullptr) ? *Value : 0.0f;
										};

	LayeringState.HeadBlendAmount				= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerHeadCurveName());
	LayeringState.HeadAdditiveBlendAmount		= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerHeadAdditiveCurveName());
	LayeringState.HeadSlotBlendAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerHeadSlotCurveName());

	LayeringState.ArmLeftBlendAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerArmLeftCurveName());
	LayeringState.ArmLeftAdditiveBlendAmount	= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerArmLeftAdditiveCurveName());
	LayeringState.ArmLeftSlotBlendAmount		= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerArmLeftSlotCurveName());
	LayeringState.ArmLeftLocalSpaceBlendAmount	= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerArmLeftLocalSpaceCurveName());
	LayeringState.ArmLeftMeshSpaceBlendAmount	= !FAnimWeight::IsFullWeight(LayeringState.ArmLeftLocalSpaceBlendAmount);

	LayeringState.ArmRightBlendAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerArmRightCurveName());
	LayeringState.ArmRightAdditiveBlendAmount	= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerArmRightAdditiveCurveName());
	LayeringState.ArmRightSlotBlendAmount		= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerArmRightSlotCurveName());
	LayeringState.ArmRightLocalSpaceBlendAmount = GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerArmRightLocalSpaceCurveName());
	LayeringState.ArmRightMeshSpaceBlendAmount	= !FAnimWeight::IsFullWeight(LayeringState.ArmRightLocalSpaceBlendAmount);

	LayeringState.HandLeftBlendAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerHandLeftCurveName());
	LayeringState.HandRightBlendAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerHandRightCurveName());

	LayeringState.SpineBlendAmount				= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerSpineCurveName());
	LayeringState.SpineAdditiveBlendAmount		= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerSpineAdditiveCurveName());
	LayeringState.SpineSlotBlendAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerSpineSlotCurveName());

	LayeringState.PelvisBlendAmount				= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerPelvisCurveName());
	LayeringState.PelvisSlotBlendAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerPelvisSlotCurveName());

	LayeringState.LegsBlendAmount				= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerLegsCurveName());
	LayeringState.LegsSlotBlendAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::LayerLegsSlotCurveName());
}

void UBECharacterAnimInstance::UpdatePose()
{
	const auto& Curves{ GetProxyOnAnyThread<FBECharacterAnimInstanceProxy>().GetAnimationCurves(EAnimCurveType::AttributeCurve) };

	// AnimCurve の値を取得するラムダ関数

	static const auto GetCurveValue = [](const TMap<FName, float>& Curves, const FName& CurveName) -> float
										{
											const float* Value = Curves.Find(CurveName);

											return (Value != nullptr) ? *Value : 0.0f;
										};

	PoseState.GroundedAmount		= GetCurveValue(Curves, UBECharacterAnimCurveNames::PoseGroundedCurveName());
	PoseState.InAirAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::PoseInAirCurveName());

	PoseState.StandingAmount		= GetCurveValue(Curves, UBECharacterAnimCurveNames::PoseStandingCurveName());
	PoseState.CrouchingAmount		= GetCurveValue(Curves, UBECharacterAnimCurveNames::PoseCrouchingCurveName());

	PoseState.MovingAmount			= GetCurveValue(Curves, UBECharacterAnimCurveNames::PoseMovingCurveName());

	PoseState.GaitAmount			= FMath::Clamp(GetCurveValue(Curves, UBECharacterAnimCurveNames::PoseGaitCurveName()), 0.0f, 3.0f);
	PoseState.GaitWalkingAmount		= UBEMovementMath::Clamp01(PoseState.GaitAmount);
	PoseState.GaitRunningAmount		= UBEMovementMath::Clamp01(PoseState.GaitAmount - 1.0f);
	PoseState.GaitSprintingAmount	= UBEMovementMath::Clamp01(PoseState.GaitAmount - 2.0f);

	// 接地ポーズ カーブの値を使用して、歩行ポーズ カーブの「ウェイトを解除」します
	// これは、接地状態への遷移の最初から完全な歩容値を瞬時に取得します

	PoseState.UnweightedGaitAmount	= (PoseState.GroundedAmount > 0.0f) ? (PoseState.GaitAmount / PoseState.GroundedAmount) : PoseState.GaitAmount;

	PoseState.UnweightedGaitWalkingAmount	= UBEMovementMath::Clamp01(PoseState.UnweightedGaitAmount);
	PoseState.UnweightedGaitRunningAmount	= UBEMovementMath::Clamp01(PoseState.UnweightedGaitAmount - 1.0f);
	PoseState.UnweightedGaitSprintingAmount = UBEMovementMath::Clamp01(PoseState.UnweightedGaitAmount - 2.0f);
}

#pragma endregion


#pragma region View

bool UBECharacterAnimInstance::IsSpineRotationAllowed()
{
	return RotationMode != TAG_Status_RotationMode_VelocityDirection;
}

void UBECharacterAnimInstance::UpdateViewOnGameThread()
{
	check(IsInGameThread());

	const auto& View{ CharacterMovement->GetViewState() };

	ViewState.Rotation = View.Rotation;
	ViewState.YawSpeed = View.YawSpeed;
}

void UBECharacterAnimInstance::UpdateView(float DeltaTime)
{
	if (!LocomotionAction.IsValid())
	{
		ViewState.YawAngle		= FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.Rotation.Yaw));
		ViewState.PitchAngle	= FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Pitch - LocomotionState.Rotation.Pitch));

		ViewState.PitchAmount	= (0.5f - ViewState.PitchAngle / 180.0f);
	}

	const auto ViewAmount{ 1.0f - GetCurveValueClamped01(UBECharacterAnimCurveNames::ViewBlockCurveName()) };
	const auto AimingAmount{ GetCurveValueClamped01(UBECharacterAnimCurveNames::AllowAimingCurveName()) };

	ViewState.LookAmount		= (ViewAmount * (1.0f - AimingAmount));

	UpdateSpineRotation(DeltaTime);

	ViewState.SpineRotation.YawAngle *= (ViewAmount * AimingAmount);
}

void UBECharacterAnimInstance::UpdateSpineRotation(float DeltaTime)
{
	auto& SpineRotation{ ViewState.SpineRotation };

	if (SpineRotation.bSpineRotationAllowed != IsSpineRotationAllowed())
	{
		SpineRotation.bSpineRotationAllowed = !SpineRotation.bSpineRotationAllowed;
		SpineRotation.StartYawAngle = SpineRotation.CurrentYawAngle;
	}

	if (SpineRotation.bSpineRotationAllowed)
	{
		static constexpr auto InterpolationSpeed{ 20.0f };

		SpineRotation.SpineAmount = bPendingUpdate ? 1.0f : UBEMovementMath::ExponentialDecay(SpineRotation.SpineAmount, 1.0f, DeltaTime, InterpolationSpeed);

		SpineRotation.TargetYawAngle = ViewState.YawAngle;
	}
	else
	{
		static constexpr auto InterpolationSpeed{ 10.0f };

		SpineRotation.SpineAmount = bPendingUpdate ? 0.0f : UBEMovementMath::ExponentialDecay(SpineRotation.SpineAmount, 0.0f, DeltaTime, InterpolationSpeed);
	}

	SpineRotation.CurrentYawAngle = UBEMovementMath::LerpAngle(SpineRotation.StartYawAngle, SpineRotation.TargetYawAngle, SpineRotation.SpineAmount);

	SpineRotation.YawAngle = SpineRotation.CurrentYawAngle;
}

void UBECharacterAnimInstance::ReinitializeLook()
{
	ViewState.Look.bReinitializationRequired = true;
}

void UBECharacterAnimInstance::UpdateLook()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UBECharacterAnimInstance::UpdateLook()"), STAT_UBECharacterAnimInstance_UpdateLook, STATGROUP_BEMovement)

	if (!IsValid(AnimData))
	{
		return;
	}

	auto& Look{ ViewState.Look };

	Look.bReinitializationRequired |= bPendingUpdate;

	const auto CharacterYawAngle{ UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw) };

	if (MovementBase.bHasRelativeRotation)
	{
		// 角度をオフセットして、移動ベースに対する相対的な角度を維持する

		Look.WorldYawAngle = FRotator3f::NormalizeAxis(Look.WorldYawAngle + MovementBase.DeltaRotation.Yaw);
	}

	float TargetYawAngle;
	float TargetPitchAngle;
	float InterpolationSpeed;

	if (RotationMode == TAG_Status_RotationMode_VelocityDirection)
	{
		// 入力方向を見るようにする

		TargetYawAngle = FRotator3f::NormalizeAxis((LocomotionState.bHasInput ? LocomotionState.InputYawAngle : LocomotionState.TargetYawAngle) - CharacterYawAngle);

		TargetPitchAngle = 0.0f;
		InterpolationSpeed = AnimData->View.LookTowardsInputYawAngleInterpolationSpeed;
	}
	else
	{
		// 視点方向を見るようにする

		TargetYawAngle = ViewState.YawAngle;
		TargetPitchAngle = ViewState.PitchAngle;
		InterpolationSpeed = AnimData->View.LookTowardsCameraRotationInterpolationSpeed;
	}

	if (Look.bReinitializationRequired || InterpolationSpeed <= 0.0f)
	{
		Look.YawAngle = TargetYawAngle;
		Look.PitchAngle = TargetPitchAngle;
	}
	else
	{
		const auto YawAngle{ FRotator3f::NormalizeAxis(Look.WorldYawAngle - CharacterYawAngle) };
		auto DeltaYawAngle{ FRotator3f::NormalizeAxis(TargetYawAngle - YawAngle) };

		if (DeltaYawAngle > 180.0f - UBEMovementMath::CounterClockwiseRotationAngleThreshold)
		{
			DeltaYawAngle -= 360.0f;
		}
		else if (FMath::Abs(LocomotionState.YawSpeed) > UE_SMALL_NUMBER && FMath::Abs(TargetYawAngle) > 90.0f)
		{
			// ヨー角を補間する場合は、最短の回転方向よりもキャラクタの回転方向を優先する。
			// 頭の回転と体の回転が同期するようにします。

			DeltaYawAngle = LocomotionState.YawSpeed > 0.0f ? FMath::Abs(DeltaYawAngle) : -FMath::Abs(DeltaYawAngle);
		}

		const auto InterpolationAmount{ UBEMovementMath::ExponentialDecay(GetDeltaSeconds(), InterpolationSpeed) };

		Look.YawAngle = FRotator3f::NormalizeAxis(YawAngle + DeltaYawAngle * InterpolationAmount);
		Look.PitchAngle = UBEMovementMath::LerpAngle(Look.PitchAngle, TargetPitchAngle, InterpolationAmount);
	}

	Look.WorldYawAngle = FRotator3f::NormalizeAxis(CharacterYawAngle + Look.YawAngle);

	// ヨー角を3つの別々の値に分ける。これらの 3 つの値は、キャラクタの周りを完全に回転させたときのビューのブレンドを改善するために使用されます。
	// キャラクタの周りを完全に回転するときのビューのブレンドを改善するために使用されます。これにより
	// ビューの応答性を維持しつつ、左から右、または右から左へスムーズにブレンドすることができます。

	Look.YawForwardAmount = Look.YawAngle / 360.0f + 0.5f;
	Look.YawLeftAmount = 0.5f - FMath::Abs(Look.YawForwardAmount - 0.5f);
	Look.YawRightAmount = 0.5f + FMath::Abs(Look.YawForwardAmount - 0.5f);

	Look.bReinitializationRequired = false;
}

#pragma endregion


#pragma region Locomotion

void UBECharacterAnimInstance::UpdateLocomotionOnGameThread()
{
	check(IsInGameThread());

	const auto& Locomotion{ CharacterMovement->GetLocomotionState() };

	LocomotionState.bHasInput				= Locomotion.bHasInput;
	LocomotionState.InputYawAngle			= Locomotion.InputYawAngle;

	LocomotionState.Speed					= Locomotion.Speed;
	LocomotionState.Velocity				= Locomotion.Velocity;
	LocomotionState.VelocityYawAngle		= Locomotion.VelocityYawAngle;
	LocomotionState.Acceleration			= Locomotion.Acceleration;

	LocomotionState.MaxAcceleration			= CharacterMovement->GetMaxAcceleration();
	LocomotionState.MaxBrakingDeceleration	= CharacterMovement->GetMaxBrakingDeceleration();
	LocomotionState.WalkableFloorZ			= CharacterMovement->GetWalkableFloorZ();

	LocomotionState.bMoving					= Locomotion.bMoving;

	LocomotionState.bMovingSmooth			= (Locomotion.bHasInput && Locomotion.bHasSpeed) || (Locomotion.Speed > AnimData->General.MovingSmoothSpeedThreshold);

	LocomotionState.TargetYawAngle			= Locomotion.TargetYawAngle;
	LocomotionState.Location				= Locomotion.Location;
	LocomotionState.Rotation				= Locomotion.Rotation;
	LocomotionState.RotationQuaternion		= Locomotion.RotationQuaternion;
	LocomotionState.YawSpeed				= Locomotion.YawSpeed;

	LocomotionState.Scale					= UE_REAL_TO_FLOAT(GetSkelMeshComponent()->GetComponentScale().Z);

	const auto* Capsule{ Character->GetCapsuleComponent() };

	LocomotionState.CapsuleRadius			= Capsule->GetScaledCapsuleRadius();
	LocomotionState.CapsuleHalfHeight		= Capsule->GetScaledCapsuleHalfHeight();
}

#pragma endregion


#pragma region OnGround

void UBECharacterAnimInstance::SetHipsDirection(EHipsDirection NewHipsDirection)
{
	GroundedState.HipsDirection = NewHipsDirection;
}

void UBECharacterAnimInstance::ActivatePivot()
{
	GroundedState.bPivotActivationRequested = true;
}

void UBECharacterAnimInstance::UpdateGroundedOnGameThread()
{
	check(IsInGameThread());

	GroundedState.bPivotActive = GroundedState.bPivotActivationRequested && !bPendingUpdate && (LocomotionState.Speed < AnimData->Grounded.PivotActivationSpeedThreshold);

	GroundedState.bPivotActivationRequested = false;
}

void UBECharacterAnimInstance::UpdateGrounded(float DeltaTime)
{
	// 常にスプリント ブロック カーブをサンプリングしてください。そうしないと、慣性ブレンディングに関する問題が発生する可能性があります。

	GroundedState.SprintBlockAmount = GetCurveValueClamped01(UBECharacterAnimCurveNames::SprintBlockCurveName());
	GroundedState.HipsDirectionLockAmount = FMath::Clamp(GetCurveValue(UBECharacterAnimCurveNames::HipsDirectionLockCurveName()), -1.0f, 1.0f);

	if (LocomotionMode != TAG_Status_LocomotionMode_OnGround)
	{
		GroundedState.VelocityBlend.bReinitializationRequired = true;
		GroundedState.SprintTime = 0.0f;
		return;
	}

	if (!LocomotionState.bMoving)
	{
		ResetGroundedLeanAmount(DeltaTime);
		return;
	}

	// 相対加速度を計算する
	// キャラクタの回転に対する相対的な加速/減速の現在の量を表します
	// この値は -1 から 1 の範囲に正規化され、-1 が最大ブレーキ減速度 
	// 1 がキャラクタ移動コンポーネントの最大加速度に等しくなります

	FVector3f RelativeAccelerationAmount;

	if ((LocomotionState.Acceleration | LocomotionState.Velocity) >= 0.0f)
	{
		RelativeAccelerationAmount = UBEMovementMath::ClampMagnitude01(
			FVector3f(LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Acceleration)) / LocomotionState.MaxAcceleration);
	}
	else
	{
		RelativeAccelerationAmount = UBEMovementMath::ClampMagnitude01(
			FVector3f(LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Acceleration)) / LocomotionState.MaxBrakingDeceleration);
	}

	UpdateMovementDirection();
	UpdateVelocityBlend(DeltaTime);
	UpdateRotationYawOffsets();

	UpdateSprint(RelativeAccelerationAmount, DeltaTime);

	UpdateStrideBlendAmount();
	UpdateWalkRunBlendAmount();

	UpdateStandingPlayRate();
	UpdateCrouchingPlayRate();

	UpdateGroundedLeanAmount(RelativeAccelerationAmount, DeltaTime);
}

void UBECharacterAnimInstance::UpdateMovementDirection()
{
	// 移動方向を計算します。この値はキャラクタのカメラに対する相対的な移動方向を表します。
	// を表し、適切な方向の状態にブレンドするためにサイクルブレンディングで使用されます。

	if (Gait == TAG_Status_Gait_Sprinting)
	{
		GroundedState.MovementDirection = EMovementDirection::Forward;
		return;
	}

	static constexpr auto ForwardHalfAngle{ 70.0f };

	GroundedState.MovementDirection = UBEMovementMath::CalculateMovementDirection(
		FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(LocomotionState.VelocityYawAngle - ViewState.Rotation.Yaw)),
		ForwardHalfAngle, 5.0f);
}

void UBECharacterAnimInstance::UpdateVelocityBlend(float DeltaTime)
{
	GroundedState.VelocityBlend.bReinitializationRequired |= bPendingUpdate;

	// ベロシティのブレンド量を計算し補間する 
	// この値は各方向のキャラクタのベロシティ量を表します
	// 標準のブレンド空間よりも優れた方向ブレンドを生成するためにブレンドマルチノードで使用されます

	const auto RelativeVelocityDirection{ FVector3f{LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Velocity)}.GetSafeNormal() };

	const auto RelativeDirection{ RelativeVelocityDirection / (FMath::Abs(RelativeVelocityDirection.X) + FMath::Abs(RelativeVelocityDirection.Y) + FMath::Abs(RelativeVelocityDirection.Z)) };

	if (GroundedState.VelocityBlend.bReinitializationRequired)
	{
		GroundedState.VelocityBlend.bReinitializationRequired = false;

		GroundedState.VelocityBlend.ForwardAmount	= UBEMovementMath::Clamp01(RelativeDirection.X);
		GroundedState.VelocityBlend.BackwardAmount	= FMath::Abs(FMath::Clamp(RelativeDirection.X, -1.0f, 0.0f));
		GroundedState.VelocityBlend.LeftAmount		= FMath::Abs(FMath::Clamp(RelativeDirection.Y, -1.0f, 0.0f));
		GroundedState.VelocityBlend.RightAmount		= UBEMovementMath::Clamp01(RelativeDirection.Y);
	}
	else
	{
		GroundedState.VelocityBlend.ForwardAmount = FMath::FInterpTo(GroundedState.VelocityBlend.ForwardAmount,
			UBEMovementMath::Clamp01(RelativeDirection.X), DeltaTime,
			AnimData->Grounded.VelocityBlendInterpolationSpeed);

		GroundedState.VelocityBlend.BackwardAmount = FMath::FInterpTo(GroundedState.VelocityBlend.BackwardAmount,
			FMath::Abs(FMath::Clamp(RelativeDirection.X, -1.0f, 0.0f)), DeltaTime,
			AnimData->Grounded.VelocityBlendInterpolationSpeed);

		GroundedState.VelocityBlend.LeftAmount = FMath::FInterpTo(GroundedState.VelocityBlend.LeftAmount,
			FMath::Abs(FMath::Clamp(RelativeDirection.Y, -1.0f, 0.0f)), DeltaTime,
			AnimData->Grounded.VelocityBlendInterpolationSpeed);

		GroundedState.VelocityBlend.RightAmount = FMath::FInterpTo(GroundedState.VelocityBlend.RightAmount,
			UBEMovementMath::Clamp01(RelativeDirection.Y), DeltaTime,
			AnimData->Grounded.VelocityBlendInterpolationSpeed);
	}
}

void UBECharacterAnimInstance::UpdateRotationYawOffsets()
{
	// 回転ヨーオフセットを設定します。これらの値は、回転ヨー オフセット カーブに影響を与えます。
	// アニメーション グラフであり、より自然な動きを実現するためにキャラクターの回転をオフセットするために使用されます。
	// カーブにより、各移動方向に対するオフセットの動作を細かく制御できます。

	const auto RotationYawOffset{ FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(LocomotionState.VelocityYawAngle - ViewState.Rotation.Yaw)) };

	GroundedState.RotationYawOffsets.ForwardAngle	= AnimData->Grounded.RotationYawOffsetForwardCurve->GetFloatValue(RotationYawOffset);
	GroundedState.RotationYawOffsets.BackwardAngle	= AnimData->Grounded.RotationYawOffsetBackwardCurve->GetFloatValue(RotationYawOffset);
	GroundedState.RotationYawOffsets.LeftAngle		= AnimData->Grounded.RotationYawOffsetLeftCurve->GetFloatValue(RotationYawOffset);
	GroundedState.RotationYawOffsets.RightAngle		= AnimData->Grounded.RotationYawOffsetRightCurve->GetFloatValue(RotationYawOffset);
}

void UBECharacterAnimInstance::UpdateSprint(const FVector3f& RelativeAccelerationAmount, float DeltaTime)
{
	if (Gait != TAG_Status_Gait_Sprinting)
	{
		GroundedState.SprintTime = 0.0f;
		GroundedState.SprintAccelerationAmount = 0.0f;
		return;
	}

	// 0.5 秒未満の場合は、相対加速度をスプリント相対加速度として使用します。
	// スプリントの開始から経過しました。それ以外の場合は、スプリントの相対加速度をゼロに設定します。
	// これは、スプリントの開始時にのみ加速アニメーションを適用するために必要です。

	static constexpr auto TimeThreshold{ 0.5f };

	GroundedState.SprintTime = bPendingUpdate ? TimeThreshold : (GroundedState.SprintTime + DeltaTime);
	GroundedState.SprintAccelerationAmount = (GroundedState.SprintTime >= TimeThreshold) ? 0.0f : RelativeAccelerationAmount.X;
}

void UBECharacterAnimInstance::UpdateStrideBlendAmount()
{
	// ストライドブレンド量を計算します。この値は、ブレンド スペース内でストライド (足の移動距離) をスケールするために使用されます。
	// キャラクターがさまざまな移動速度で歩いたり走ったりできるようにします。また、歩いたり走ったりする歩行アニメーションも可能になります。
	// アニメーション速度を移動速度に合わせながら個別にブレンドし、キャラクターが必要とするのを防ぎます。
	// 半分ウォーク + 半分ランのブレンドを再生します。曲線は、ストライド量を速度にマッピングして最大限の制御を実現するために使用されます。

	const auto Speed{ LocomotionState.Speed / LocomotionState.Scale };

	const auto StandingStrideBlend{ FMath::Lerp(AnimData->Grounded.StrideBlendAmountWalkCurve->GetFloatValue(Speed),
												  AnimData->Grounded.StrideBlendAmountRunCurve->GetFloatValue(Speed),
												  PoseState.UnweightedGaitRunningAmount)
	};

	// しゃがんだ歩幅のブレンド量。

	GroundedState.StrideBlendAmount = FMath::Lerp(StandingStrideBlend, AnimData->Grounded.StrideBlendAmountWalkCurve->GetFloatValue(Speed), PoseState.CrouchingAmount);
}

void UBECharacterAnimInstance::UpdateWalkRunBlendAmount()
{
	// Walk / Run ブレンド量を計算します。この値は、ウォーキングとランニングをブレンドするためにブレンド スペース内で使用されます。

	GroundedState.WalkRunBlendAmount = (Gait == TAG_Status_Gait_Walking) ? 0.0f : 1.0f;
}

void UBECharacterAnimInstance::UpdateStandingPlayRate()
{
	// キャラクターの速度を各歩行のアニメートされた速度で割ることにより、スタンディング プレイ レートを計算します。
	// 補間は、すべての移動サイクルに存在する歩行量カーブによって決定されるため、
	// 再生レートは、現在ブレンドされているアニメーションと常に同期します。値も次の値で除算されます。
	// ストライドまたはスケールが小さくなるにつれて再生レートが増加するように、ストライド ブレンドとカプセル スケールを調整します。

	const auto WalkRunSpeedAmount{ FMath::Lerp(LocomotionState.Speed / AnimData->Grounded.AnimatedWalkSpeed,
												 LocomotionState.Speed / AnimData->Grounded.AnimatedRunSpeed,
												 PoseState.UnweightedGaitRunningAmount)
	};

	const auto WalkRunSprintSpeedAmount{ FMath::Lerp(WalkRunSpeedAmount,
													   LocomotionState.Speed / AnimData->Grounded.AnimatedSprintSpeed,
													   PoseState.UnweightedGaitSprintingAmount)
	};

	GroundedState.StandingPlayRate = FMath::Clamp(WalkRunSprintSpeedAmount / (GroundedState.StrideBlendAmount * LocomotionState.Scale), 0.0f, 3.0f);
}

void UBECharacterAnimInstance::UpdateCrouchingPlayRate()
{
	// キャラクターの速度をアニメートされた速度で割ることにより、しゃがみプレイ レートを計算します。この値には次のことが必要です
	// 動作中のしゃがみから立ちまでのブレンドを改善するために、立ちプレイ レートとは別にします。

	GroundedState.CrouchingPlayRate = FMath::Clamp(LocomotionState.Speed / (AnimData->Grounded.AnimatedCrouchSpeed * GroundedState.StrideBlendAmount * LocomotionState.Scale), 0.0f, 2.0f);
}

void UBECharacterAnimInstance::UpdateGroundedLeanAmount(const FVector3f& RelativeAccelerationAmount, float DeltaTime)
{
	if (bPendingUpdate)
	{
		LeanState.RightAmount = RelativeAccelerationAmount.Y;
		LeanState.ForwardAmount = RelativeAccelerationAmount.X;
	}
	else
	{
		LeanState.RightAmount = FMath::FInterpTo(LeanState.RightAmount, RelativeAccelerationAmount.Y, DeltaTime, AnimData->General.LeanInterpolationSpeed);
		LeanState.ForwardAmount = FMath::FInterpTo(LeanState.ForwardAmount, RelativeAccelerationAmount.X, DeltaTime, AnimData->General.LeanInterpolationSpeed);
	}
}

void UBECharacterAnimInstance::ResetGroundedLeanAmount(float DeltaTime)
{
	if (bPendingUpdate)
	{
		LeanState.RightAmount = 0.0f;
		LeanState.ForwardAmount = 0.0f;
	}
	else
	{
		LeanState.RightAmount = FMath::FInterpTo(LeanState.RightAmount, 0.0f, DeltaTime, AnimData->General.LeanInterpolationSpeed);
		LeanState.ForwardAmount = FMath::FInterpTo(LeanState.ForwardAmount, 0.0f, DeltaTime, AnimData->General.LeanInterpolationSpeed);
	}
}

#pragma endregion


#pragma region InAir

void UBECharacterAnimInstance::ResetJumped()
{
	InAirState.bJumped = false;
}

void UBECharacterAnimInstance::UpdateInAirOnGameThread()
{
	check(IsInGameThread());

	InAirState.bJumped = !bPendingUpdate && (InAirState.bJumped || (InAirState.VerticalVelocity > 0));
}

void UBECharacterAnimInstance::UpdateInAir(float DeltaTime)
{
	if (LocomotionMode != TAG_Status_LocomotionMode_InAir)
	{
		return;
	}

	if (InAirState.bJumped)
	{
		static constexpr auto ReferenceSpeed{ 600.0f };
		static constexpr auto MinPlayRate{ 1.2f };
		static constexpr auto MaxPlayRate{ 1.5f };

		InAirState.JumpPlayRate = UBEMovementMath::LerpClamped(MinPlayRate, MaxPlayRate, LocomotionState.Speed / ReferenceSpeed);
	}

	// 垂直速度をキャッシュしキャラクターが地面に着地する速度が決定する

	InAirState.VerticalVelocity = UE_REAL_TO_FLOAT(LocomotionState.Velocity.Z);

	UpdateGroundPredictionAmount();

	UpdateInAirLeanAmount(DeltaTime);
}

void UBECharacterAnimInstance::UpdateGroundPredictionAmount()
{
	// 速度方向にトレースして、キャラクターが歩行可能なサーフェスを見つけることによって、地面の予測ウェイトを計算します。
	// に向かって落下し、衝突するまでの「時間」(0 から 1 の範囲、1 が最大、0 が接地寸前) を取得します。
	// 地面予測量カーブは、滑らかなブレンドの最終量に時間がどのように影響するかを制御するために使用されます。

	static constexpr auto VerticalVelocityThreshold{ -200.0f };
	static constexpr auto MinVerticalVelocity{ -4000.0f };
	static constexpr auto MaxVerticalVelocity{ -200.0f };
	static constexpr auto MinSweepDistance{ 150.0f };
	static constexpr auto MaxSweepDistance{ 2000.0f };

	if (InAirState.VerticalVelocity > VerticalVelocityThreshold)
	{
		InAirState.GroundPredictionAmount = 0.0f;
		return;
	}

	const auto AllowanceAmount{ 1.0f - GetCurveValueClamped01(UBECharacterAnimCurveNames::GroundPredictionBlockCurveName()) };

	if (AllowanceAmount <= UE_KINDA_SMALL_NUMBER)
	{
		InAirState.GroundPredictionAmount = 0.0f;
		return;
	}

	const auto SweepStartLocation{ LocomotionState.Location };

	auto VelocityDirection{ LocomotionState.Velocity };

	VelocityDirection.Z = FMath::Clamp(VelocityDirection.Z, MinVerticalVelocity, MaxVerticalVelocity);
	VelocityDirection.Normalize();

	const auto SweepVector{ VelocityDirection * FMath::GetMappedRangeValueClamped(FVector2f(MaxVerticalVelocity, MinVerticalVelocity), FVector2f(MinSweepDistance, MaxSweepDistance), InAirState.VerticalVelocity) * LocomotionState.Scale };

	FHitResult Hit;
	GetWorld()->SweepSingleByChannel(
		Hit, 
		SweepStartLocation, 
		SweepStartLocation + SweepVector, 
		FQuat::Identity, 
		ECC_WorldStatic,
		FCollisionShape::MakeCapsule(LocomotionState.CapsuleRadius, LocomotionState.CapsuleHalfHeight),
		FCollisionQueryParams(__FUNCTION__, false, Character), 
		AnimData->InAir.GroundPredictionSweepResponses);
	
	const auto bGroundValid{ Hit.IsValidBlockingHit() && (Hit.ImpactNormal.Z >= LocomotionState.WalkableFloorZ) };

	InAirState.GroundPredictionAmount = bGroundValid ? (AnimData->InAir.GroundPredictionAmountCurve->GetFloatValue(Hit.Time) * AllowanceAmount) : 0.0f;
}

void UBECharacterAnimInstance::UpdateInAirLeanAmount(float DeltaTime)
{
	// 相対速度の方向と量を使用して、キャラクターがどれだけ傾くかを決定します
	// 空中にいるとき。傾斜量カーブは垂直速度を取得し、乗数として使用されます。
	// 上向きの移動から下向きの移動に移行するときに、傾斜方向をスムーズに反転します。

	static constexpr auto ReferenceSpeed{ 350.0f };

	const auto RelativeVelocity{ FVector3f(LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Velocity)) /
								 ReferenceSpeed * AnimData->InAir.LeanAmountCurve->GetFloatValue(InAirState.VerticalVelocity)
	};

	if (bPendingUpdate)
	{
		LeanState.RightAmount = RelativeVelocity.Y;
		LeanState.ForwardAmount = RelativeVelocity.X;
	}
	else
	{
		LeanState.RightAmount = FMath::FInterpTo(LeanState.RightAmount, RelativeVelocity.Y,
			DeltaTime, AnimData->General.LeanInterpolationSpeed);

		LeanState.ForwardAmount = FMath::FInterpTo(LeanState.ForwardAmount, RelativeVelocity.X,
			DeltaTime, AnimData->General.LeanInterpolationSpeed);
	}
}

#pragma endregion


#pragma region Feet

void UBECharacterAnimInstance::UpdateFeetOnGameThread()
{
	check(IsInGameThread());

	const auto* Mesh{ GetSkelMeshComponent() };

	const auto FootLeftTargetTransform{
		Mesh->GetSocketTransform(AnimData->General.bUseFootIkBones
									 ? UBECharacterAnimCurveNames::FootLeftIkBoneName()
									 : UBECharacterAnimCurveNames::FootLeftVirtualBoneName())
	};

	FeetState.Left.TargetLocation = FootLeftTargetTransform.GetLocation();
	FeetState.Left.TargetRotation = FootLeftTargetTransform.GetRotation();

	const auto FootRightTargetTransform{
		Mesh->GetSocketTransform(AnimData->General.bUseFootIkBones
									 ? UBECharacterAnimCurveNames::FootRightIkBoneName()
									 : UBECharacterAnimCurveNames::FootRightVirtualBoneName())
	};

	FeetState.Right.TargetLocation = FootRightTargetTransform.GetLocation();
	FeetState.Right.TargetRotation = FootRightTargetTransform.GetRotation();
}

void UBECharacterAnimInstance::UpdateFeet(float DeltaTime)
{
	FeetState.FootPlantedAmount = FMath::Clamp(GetCurveValue(UBECharacterAnimCurveNames::FootPlantedCurveName()), -1.0f, 1.0f);
	FeetState.FeetCrossingAmount = GetCurveValueClamped01(UBECharacterAnimCurveNames::FeetCrossingCurveName());

	FeetState.MinMaxPelvisOffsetZ = FVector2D::ZeroVector;

	const auto ComponentTransformInverse{ GetProxyOnAnyThread<FAnimInstanceProxy>().GetComponentTransform().Inverse() };

	UpdateFoot(FeetState.Left, UBECharacterAnimCurveNames::FootLeftIkCurveName(),
								UBECharacterAnimCurveNames::FootLeftLockCurveName(), ComponentTransformInverse, DeltaTime);

	UpdateFoot(FeetState.Right, UBECharacterAnimCurveNames::FootRightIkCurveName(),
								 UBECharacterAnimCurveNames::FootRightLockCurveName(), ComponentTransformInverse, DeltaTime);

	FeetState.MinMaxPelvisOffsetZ.X = FMath::Min(FeetState.Left.OffsetTargetLocation.Z, FeetState.Right.OffsetTargetLocation.Z) /
												 LocomotionState.Scale;

	FeetState.MinMaxPelvisOffsetZ.Y = FMath::Max(FeetState.Left.OffsetTargetLocation.Z, FeetState.Right.OffsetTargetLocation.Z) /
												 LocomotionState.Scale;
}

void UBECharacterAnimInstance::UpdateFoot(FFootState& FootState, const FName& FootIkCurveName, const FName& FootLockCurveName, const FTransform& ComponentTransformInverse, float DeltaTime) const
{
	FootState.IkAmount = GetCurveValueClamped01(FootIkCurveName);

	ProcessFootLockTeleport(FootState);

	ProcessFootLockBaseChange(FootState, ComponentTransformInverse);

	auto FinalLocation{ FootState.TargetLocation };
	auto FinalRotation{ FootState.TargetRotation };

	UpdateFootLock(FootState, FootLockCurveName, ComponentTransformInverse, DeltaTime, FinalLocation, FinalRotation);

	UpdateFootOffset(FootState, DeltaTime, FinalLocation, FinalRotation);

	FootState.IkLocation = ComponentTransformInverse.TransformPosition(FinalLocation);
	FootState.IkRotation = ComponentTransformInverse.TransformRotation(FinalRotation);
}

void UBECharacterAnimInstance::ProcessFootLockTeleport(FFootState& FootState) const
{
	// ネットワークの平滑化により、テレポートは短時間で発生すると仮定します。
	// テレポーテーション イベントを受け入れた後もキャラクターは 1 フレーム内で移動できるため、
	// 一定の時間はかかるため、フット ロックの不具合を避けるためにこれを考慮する必要があります。

	if (bPendingUpdate || GetWorld()->TimeSince(TeleportedTime) > 0.2f ||
		!FAnimWeight::IsRelevant(FootState.IkAmount * FootState.LockAmount))
	{
		return;
	}

	const auto& ComponentTransform{ GetProxyOnAnyThread<FAnimInstanceProxy>().GetComponentTransform() };

	FootState.LockLocation = ComponentTransform.TransformPosition(FootState.LockComponentRelativeLocation);
	FootState.LockRotation = ComponentTransform.TransformRotation(FootState.LockComponentRelativeRotation);

	if (MovementBase.bHasRelativeLocation)
	{
		const auto BaseRotationInverse{ MovementBase.Rotation.Inverse() };

		FootState.LockMovementBaseRelativeLocation = BaseRotationInverse.RotateVector(FootState.LockLocation - MovementBase.Location);
		FootState.LockMovementBaseRelativeRotation = BaseRotationInverse * FootState.LockRotation;
	}
}

void UBECharacterAnimInstance::ProcessFootLockBaseChange(FFootState& FootState, const FTransform& ComponentTransformInverse) const
{
	if ((!bPendingUpdate && !MovementBase.bBaseChanged) || !FAnimWeight::IsRelevant(FootState.IkAmount * FootState.LockAmount))
	{
		return;
	}

	if (bPendingUpdate)
	{
		FootState.LockLocation = FootState.TargetLocation;
		FootState.LockRotation = FootState.TargetRotation;
	}

	FootState.LockComponentRelativeLocation = ComponentTransformInverse.TransformPosition(FootState.LockLocation);
	FootState.LockComponentRelativeRotation = ComponentTransformInverse.TransformRotation(FootState.LockRotation);

	if (MovementBase.bHasRelativeLocation)
	{
		const auto BaseRotationInverse{ MovementBase.Rotation.Inverse() };

		FootState.LockMovementBaseRelativeLocation = BaseRotationInverse.RotateVector(FootState.LockLocation - MovementBase.Location);
		FootState.LockMovementBaseRelativeRotation = BaseRotationInverse * FootState.LockRotation;
	}
	else
	{
		FootState.LockMovementBaseRelativeLocation = FVector::ZeroVector;
		FootState.LockMovementBaseRelativeRotation = FQuat::Identity;
	}
}

void UBECharacterAnimInstance::UpdateFootLock(FFootState& FootState, const FName& FootLockCurveName, const FTransform& ComponentTransformInverse, float DeltaTime, FVector& FinalLocation, FQuat& FinalRotation) const
{
	auto NewFootLockAmount{ GetCurveValueClamped01(FootLockCurveName) };

	NewFootLockAmount *= 1.0f - RotateInPlaceState.FootLockBlockAmount;

	if (LocomotionState.bMovingSmooth || LocomotionMode != TAG_Status_LocomotionMode_OnGround)
	{
		// キャラクターが移動中または空中にいる場合、足のロックをスムーズに無効にします。
		// アニメーション ブループリントのカーブ値に依存する代わりに。

		static constexpr auto MovingDecreaseSpeed{ 5.0f };
		static constexpr auto NotGroundedDecreaseSpeed{ 0.6f };

		NewFootLockAmount = bPendingUpdate
			? 0.0f
			: FMath::Max(
				0.0f, 
				FMath::Min(
					NewFootLockAmount,
					FootState.LockAmount - DeltaTime * (LocomotionState.bMovingSmooth ? MovingDecreaseSpeed : NotGroundedDecreaseSpeed)
				)
			);
	}

	if (AnimData->Feet.bDisableFootLock || !FAnimWeight::IsRelevant(FootState.IkAmount * NewFootLockAmount))
	{
		if (FootState.LockAmount > 0.0f)
		{
			FootState.LockAmount = 0.0f;

			FootState.LockLocation = FVector::ZeroVector;
			FootState.LockRotation = FQuat::Identity;

			FootState.LockComponentRelativeLocation = FVector::ZeroVector;
			FootState.LockComponentRelativeRotation = FQuat::Identity;

			FootState.LockMovementBaseRelativeLocation = FVector::ZeroVector;
			FootState.LockMovementBaseRelativeRotation = FQuat::Identity;
		}

		return;
	}

	const auto bNewAmountEqualOne{ FAnimWeight::IsFullWeight(NewFootLockAmount) };
	const auto bNewAmountGreaterThanPrevious{ NewFootLockAmount > FootState.LockAmount };

	// 新しい量が現在の量より小さいか 1 に等しい場合にのみ、フット ロック量を更新します。これは
	// 足をロックされた位置からブレンドアウトしたり、新しい位置にロックしたりできますが、ブレンドすることはできません。

	if (bNewAmountEqualOne)
	{
		if (bNewAmountGreaterThanPrevious)
		{
			// 新しいフット ロック量が 1 で、以前の量が 1 未満の場合は、新しいフット ロックの位置と回転を保存します。

			if (FootState.LockAmount <= 0.9f)
			{
				// 前回のロック時と同じロック位置と回転を維持します。
				// 足の「テレポート」の問題を取り除くために、量は 1 に近くなります。

				FootState.LockLocation = FinalLocation;
				FootState.LockRotation = FinalRotation;
			}

			if (MovementBase.bHasRelativeLocation)
			{
				const auto BaseRotationInverse{ MovementBase.Rotation.Inverse() };

				FootState.LockMovementBaseRelativeLocation = BaseRotationInverse.RotateVector(FinalLocation - MovementBase.Location);
				FootState.LockMovementBaseRelativeRotation = BaseRotationInverse * FinalRotation;
			}
			else
			{
				FootState.LockMovementBaseRelativeLocation = FVector::ZeroVector;
				FootState.LockMovementBaseRelativeRotation = FQuat::Identity;
			}
		}

		FootState.LockAmount = 1.0f;
	}
	else if (!bNewAmountGreaterThanPrevious)
	{
		FootState.LockAmount = NewFootLockAmount;
	}

	if (MovementBase.bHasRelativeLocation)
	{
		FootState.LockLocation = MovementBase.Location + MovementBase.Rotation.RotateVector(FootState.LockMovementBaseRelativeLocation);
		FootState.LockRotation = MovementBase.Rotation * FootState.LockMovementBaseRelativeRotation;
	}

	FootState.LockComponentRelativeLocation = ComponentTransformInverse.TransformPosition(FootState.LockLocation);
	FootState.LockComponentRelativeRotation = ComponentTransformInverse.TransformRotation(FootState.LockRotation);

	FinalLocation = FMath::Lerp(FinalLocation, FootState.LockLocation, FootState.LockAmount);
	FinalRotation = FQuat::Slerp(FinalRotation, FootState.LockRotation, FootState.LockAmount);
}

void UBECharacterAnimInstance::UpdateFootOffset(FFootState& FootState, float DeltaTime, FVector& FinalLocation, FQuat& FinalRotation) const
{
	if (!FAnimWeight::IsRelevant(FootState.IkAmount))
	{
		FootState.OffsetTargetLocation = FVector::ZeroVector;
		FootState.OffsetTargetRotation = FQuat::Identity;
		FootState.OffsetSpringState.Reset();
		return;
	}

	if (LocomotionMode == TAG_Status_LocomotionMode_InAir)
	{
		FootState.OffsetTargetLocation = FVector::ZeroVector;
		FootState.OffsetTargetRotation = FQuat::Identity;
		FootState.OffsetSpringState.Reset();

		if (bPendingUpdate)
		{
			FootState.OffsetLocation = FVector::ZeroVector;
			FootState.OffsetRotation = FQuat::Identity;
		}
		else
		{
			static constexpr auto InterpolationSpeed{ 15.0f };

			FootState.OffsetLocation = FMath::VInterpTo(FootState.OffsetLocation, FVector::ZeroVector, DeltaTime, InterpolationSpeed);
			FootState.OffsetRotation = FMath::QInterpTo(FootState.OffsetRotation, FQuat::Identity, DeltaTime, InterpolationSpeed);

			FinalLocation += FootState.OffsetLocation;
			FinalRotation = FootState.OffsetRotation * FinalRotation;
		}

		return;
	}

	// 足の位置から下に向かってトレースして、ジオメトリを見つけます。サーフェスが歩行可能な場合は、衝撃の位置と法線を保存する

	const FVector TraceLocation{ FinalLocation.X, FinalLocation.Y, GetProxyOnAnyThread<FAnimInstanceProxy>().GetComponentTransform().GetLocation().Z };

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceLocation + FVector(0.0f, 0.0f, AnimData->Feet.IkTraceDistanceUpward* LocomotionState.Scale),
		TraceLocation - FVector(0.0f, 0.0f, AnimData->Feet.IkTraceDistanceDownward * LocomotionState.Scale),
		UEngineTypes::ConvertToCollisionChannel(AnimData->Feet.IkTraceChannel),
		FCollisionQueryParams(__FUNCTION__, true, Character));

	const auto bGroundValid{ Hit.IsValidBlockingHit() && Hit.ImpactNormal.Z >= LocomotionState.WalkableFloorZ };

	if (bGroundValid)
	{
		const auto FootHeight{ AnimData->Feet.FootHeight * LocomotionState.Scale };

		// 衝撃の位置と予想される (平らな) 床の位置の間の位置の違いを見つけます。これらは
		// 角度のあるサーフェスでの動作を改善するために、値は衝撃法線に足の高さを乗じたものによってオフセットされます。

		FootState.OffsetTargetLocation = Hit.ImpactPoint - TraceLocation + Hit.ImpactNormal * FootHeight;
		FootState.OffsetTargetLocation.Z -= FootHeight;

		// 回転オフセットを計算する

		FootState.OffsetTargetRotation = FRotator(
			-UBEMovementMath::DirectionToAngle(FVector2D(Hit.ImpactNormal.Z, Hit.ImpactNormal.X)),
			0.0f,
			UBEMovementMath::DirectionToAngle(FVector2D(Hit.ImpactNormal.Z, Hit.ImpactNormal.Y))).Quaternion();
	}

	// 現在のオフセットを新しいターゲット値に補間する

	if (bPendingUpdate)
	{
		FootState.OffsetSpringState.Reset();

		FootState.OffsetLocation = FootState.OffsetTargetLocation;
		FootState.OffsetRotation = FootState.OffsetTargetRotation;
	}
	else
	{
		static constexpr auto LocationInterpolationFrequency{ 0.4f };
		static constexpr auto LocationInterpolationDampingRatio{ 4.0f };
		static constexpr auto LocationInterpolationTargetVelocityAmount{ 1.0f };

		FootState.OffsetLocation = UBEMovementMath::SpringDampVector(FootState.OffsetLocation, FootState.OffsetTargetLocation,
			FootState.OffsetSpringState, DeltaTime, LocationInterpolationFrequency,
			LocationInterpolationDampingRatio, LocationInterpolationTargetVelocityAmount);

		static constexpr auto RotationInterpolationSpeed{ 30.0f };

		FootState.OffsetRotation = FMath::QInterpTo(FootState.OffsetRotation, FootState.OffsetTargetRotation,
			DeltaTime, RotationInterpolationSpeed);
	}

	FinalLocation += FootState.OffsetLocation;
	FinalRotation = FootState.OffsetRotation * FinalRotation;
}

#pragma endregion


#pragma region Transitions

void UBECharacterAnimInstance::PlayQuickStopAnimation()
{
	if (RotationMode != TAG_Status_RotationMode_VelocityDirection)
	{
		PlayTransitionLeftAnimation(AnimData->Transitions.QuickStopBlendInDuration, AnimData->Transitions.QuickStopBlendOutDuration,
			AnimData->Transitions.QuickStopPlayRate.X, AnimData->Transitions.QuickStopStartTime);
		return;
	}

	auto RotationYawAngle{ FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT((LocomotionState.bHasInput ? LocomotionState.InputYawAngle : LocomotionState.TargetYawAngle) - LocomotionState.Rotation.Yaw)) };

	if (RotationYawAngle > 180.0f - UBEMovementMath::CounterClockwiseRotationAngleThreshold)
	{
		RotationYawAngle -= 360.0f;
	}

	// キャラクターの距離に基づいてクイック ストップ アニメーションの再生速度を調整します
	// 回転していきます。 180 度では、再生レートが最大になります。

	if (RotationYawAngle <= 0.0f)
	{
		PlayTransitionLeftAnimation(AnimData->Transitions.QuickStopBlendInDuration, AnimData->Transitions.QuickStopBlendOutDuration,
			FMath::Lerp(AnimData->Transitions.QuickStopPlayRate.X, AnimData->Transitions.QuickStopPlayRate.Y,
				FMath::Abs(RotationYawAngle) / 180.0f), AnimData->Transitions.QuickStopStartTime);
	}
	else
	{
		PlayTransitionRightAnimation(AnimData->Transitions.QuickStopBlendInDuration, AnimData->Transitions.QuickStopBlendOutDuration,
			FMath::Lerp(AnimData->Transitions.QuickStopPlayRate.X, AnimData->Transitions.QuickStopPlayRate.Y,
				FMath::Abs(RotationYawAngle) / 180.0f), AnimData->Transitions.QuickStopStartTime);
	}
}

void UBECharacterAnimInstance::PlayTransitionAnimation(UAnimSequenceBase* Animation, float BlendInDuration, float BlendOutDuration, float PlayRate, float StartTime, bool bFromStandingIdleOnly)
{
	check(IsInGameThread());

	if (!IsValid(CharacterMovement))
	{
		return;
	}

	if (bFromStandingIdleOnly && (CharacterMovement->GetLocomotionState().bMoving || CharacterMovement->GetStance() != TAG_Status_Stance_Standing))
	{
		return;
	}

	PlaySlotAnimationAsDynamicMontage(Animation, UBECharacterAnimCurveNames::TransitionSlotName(), BlendInDuration, BlendOutDuration, PlayRate, 1, 0.0f, StartTime);
}

void UBECharacterAnimInstance::PlayTransitionLeftAnimation(float BlendInDuration, float BlendOutDuration, float PlayRate, float StartTime, bool bFromStandingIdleOnly)
{
	if (!IsValid(AnimData))
	{
		return;
	}

	PlayTransitionAnimation(Stance == TAG_Status_Stance_Crouching
		? AnimData->Transitions.CrouchingTransitionLeftAnimation
		: AnimData->Transitions.StandingTransitionLeftAnimation,
		BlendInDuration, BlendOutDuration, PlayRate, StartTime, bFromStandingIdleOnly);
}

void UBECharacterAnimInstance::PlayTransitionRightAnimation(float BlendInDuration, float BlendOutDuration, float PlayRate, float StartTime, bool bFromStandingIdleOnly)
{
	if (!IsValid(AnimData))
	{
		return;
	}

	PlayTransitionAnimation(Stance == TAG_Status_Stance_Crouching
		? AnimData->Transitions.CrouchingTransitionRightAnimation
		: AnimData->Transitions.StandingTransitionRightAnimation,
		BlendInDuration, BlendOutDuration, PlayRate, StartTime, bFromStandingIdleOnly);
}

void UBECharacterAnimInstance::StopTransitionAndTurnInPlaceAnimations(float BlendOutDuration)
{
	check(IsInGameThread());

	StopSlotAnimation(BlendOutDuration, UBECharacterAnimCurveNames::TransitionSlotName());
}

void UBECharacterAnimInstance::UpdateTransitions()
{
	// 許可遷移曲線は特定の状態内で変更されるため、それらの状態では許可される遷移が true になります。

	TransitionsState.bTransitionsAllowed = FAnimWeight::IsFullWeight(GetCurveValue(UBECharacterAnimCurveNames::AllowTransitionsCurveName()));

	UpdateDynamicTransition();
}

void UBECharacterAnimInstance::UpdateDynamicTransition()
{
	if (TransitionsState.DynamicTransitionsFrameDelay > 0)
	{
		TransitionsState.DynamicTransitionsFrameDelay -= 1;
		return;
	}

	if (!TransitionsState.bTransitionsAllowed || LocomotionState.bMoving || LocomotionMode != TAG_Status_LocomotionMode_OnGround)
	{
		return;
	}

	// 各足をチェックして、足の外観とその望ましい/ターゲットの位置との位置の違いを確認します。
	// しきい値を超えています。そうなった場合は、その足で追加のトランジション アニメーションを再生します。現在設定されている
	// トランジションは 2 足のトランジション アニメーションの後半を再生し、1 つの足だけが動きます。

	const auto FootLockDistanceThresholdSquared{ FMath::Square(AnimData->Transitions.DynamicTransitionFootLockDistanceThreshold * LocomotionState.Scale) };

	const auto FootLockLeftDistanceSquared{ FVector::DistSquared(FeetState.Left.TargetLocation, FeetState.Left.LockLocation) };
	const auto FootLockRightDistanceSquared{ FVector::DistSquared(FeetState.Right.TargetLocation, FeetState.Right.LockLocation) };

	const auto bTransitionLeftAllowed{ FAnimWeight::IsRelevant(FeetState.Left.LockAmount) && FootLockLeftDistanceSquared > FootLockDistanceThresholdSquared };
	const auto bTransitionRightAllowed{ FAnimWeight::IsRelevant(FeetState.Right.LockAmount) && FootLockRightDistanceSquared > FootLockDistanceThresholdSquared };

	if (!bTransitionLeftAllowed && !bTransitionRightAllowed)
	{
		return;
	}

	TObjectPtr<UAnimSequenceBase> DynamicTransitionAnimation;

	// 両方の遷移が許可される場合は、ロック距離が大きい方を選択します。

	if (!bTransitionLeftAllowed)
	{
		DynamicTransitionAnimation = Stance == TAG_Status_Stance_Crouching
			? AnimData->Transitions.CrouchingDynamicTransitionRightAnimation
			: AnimData->Transitions.StandingDynamicTransitionRightAnimation;
	}
	else if (!bTransitionRightAllowed)
	{
		DynamicTransitionAnimation = Stance == TAG_Status_Stance_Crouching
			? AnimData->Transitions.CrouchingDynamicTransitionLeftAnimation
			: AnimData->Transitions.StandingDynamicTransitionLeftAnimation;
	}
	else if (FootLockLeftDistanceSquared >= FootLockRightDistanceSquared)
	{
		DynamicTransitionAnimation = Stance == TAG_Status_Stance_Crouching
			? AnimData->Transitions.CrouchingDynamicTransitionLeftAnimation
			: AnimData->Transitions.StandingDynamicTransitionLeftAnimation;
	}
	else
	{
		DynamicTransitionAnimation = Stance == TAG_Status_Stance_Crouching
			? AnimData->Transitions.CrouchingDynamicTransitionRightAnimation
			: AnimData->Transitions.StandingDynamicTransitionRightAnimation;
	}

	if (IsValid(DynamicTransitionAnimation))
	{
		// 次の動的トランジションを約 2 フレームブロックして、アニメーション ブループリントがアニメーションに適切に反応する時間を与えます。

		TransitionsState.DynamicTransitionsFrameDelay = 2;

		// アニメーション モンタージュはワーカー スレッドでは再生できないため、キューに入れて後でゲーム スレッドで再生します。

		TransitionsState.QueuedDynamicTransitionAnimation = DynamicTransitionAnimation;

		if (IsInGameThread())
		{
			PlayQueuedDynamicTransitionAnimation();
		}
	}
}

void UBECharacterAnimInstance::PlayQueuedDynamicTransitionAnimation()
{
	check(IsInGameThread());

	PlaySlotAnimationAsDynamicMontage(TransitionsState.QueuedDynamicTransitionAnimation, UBECharacterAnimCurveNames::TransitionSlotName(),
		AnimData->Transitions.DynamicTransitionBlendDuration,
		AnimData->Transitions.DynamicTransitionBlendDuration,
		AnimData->Transitions.DynamicTransitionPlayRate, 1, 0.0f);

	TransitionsState.QueuedDynamicTransitionAnimation = nullptr;
}

#pragma endregion


#pragma region Rotate In Place

bool UBECharacterAnimInstance::IsRotateInPlaceAllowed()
{
	return RotationMode != TAG_Status_RotationMode_VelocityDirection;
}

void UBECharacterAnimInstance::UpdateRotateInPlace(float DeltaTime)
{
	static constexpr auto PlayRateInterpolationSpeed{ 5.0f };

	// その場で回転は、キャラクターが静止して照準を合わせている場合、または一人称視点モードの場合にのみ許可されます。

	if (LocomotionState.bMoving || LocomotionMode != TAG_Status_LocomotionMode_OnGround || !IsRotateInPlaceAllowed())
	{
		RotateInPlaceState.bRotatingLeft = false;
		RotateInPlaceState.bRotatingRight = false;

		RotateInPlaceState.PlayRate = bPendingUpdate
			? AnimData->RotateInPlace.PlayRate.X
			: FMath::FInterpTo(RotateInPlaceState.PlayRate, AnimData->RotateInPlace.PlayRate.X,
				DeltaTime, PlayRateInterpolationSpeed);

		RotateInPlaceState.FootLockBlockAmount = 0.0f;
		return;
	}

	// ビューのヨー角がしきい値を超えているかどうかを確認して、キャラクターを左または右に回転させるかどうかを確認します。

	RotateInPlaceState.bRotatingLeft = ViewState.YawAngle < -AnimData->RotateInPlace.ViewYawAngleThreshold;
	RotateInPlaceState.bRotatingRight = ViewState.YawAngle > AnimData->RotateInPlace.ViewYawAngleThreshold;

	if (!RotateInPlaceState.bRotatingLeft && !RotateInPlaceState.bRotatingRight)
	{
		RotateInPlaceState.PlayRate = bPendingUpdate
			? AnimData->RotateInPlace.PlayRate.X
			: FMath::FInterpTo(RotateInPlaceState.PlayRate, AnimData->RotateInPlace.PlayRate.X,
				DeltaTime, PlayRateInterpolationSpeed);

		RotateInPlaceState.FootLockBlockAmount = 0.0f;
		return;
	}

	// キャラクターを回転させる必要がある場合は、ビューのヨーに合わせて再生レートを設定します。
	// スピード。これにより、カメラを速く動かすとキャラクターの回転も速くなります。

	const auto PlayRate{ FMath::GetMappedRangeValueClamped(AnimData->RotateInPlace.ReferenceViewYawSpeed, AnimData->RotateInPlace.PlayRate, ViewState.YawSpeed) };

	RotateInPlaceState.PlayRate = bPendingUpdate
		? PlayRate
		: FMath::FInterpTo(RotateInPlaceState.PlayRate, PlayRate,
			DeltaTime, PlayRateInterpolationSpeed);

	// 大きな角度で回転する場合、または回転が速すぎる場合は、フット ロックを無効にします。そうしないと、脚がらせん状にねじれる可能性があります。

	static constexpr auto BlockInterpolationSpeed{ 5.0f };

	RotateInPlaceState.FootLockBlockAmount =
		AnimData->RotateInPlace.bDisableFootLock
		? 1.0f
		: FMath::Abs(ViewState.YawAngle) > AnimData->RotateInPlace.FootLockBlockViewYawAngleThreshold
		? 0.5f
		: ViewState.YawSpeed <= AnimData->RotateInPlace.FootLockBlockViewYawSpeedThreshold
		? 0.0f
		: bPendingUpdate
		? 1.0f
		: FMath::FInterpTo(RotateInPlaceState.FootLockBlockAmount, 1.0f, DeltaTime, BlockInterpolationSpeed);
}

#pragma endregion


#pragma region Utilities

float UBECharacterAnimInstance::GetCurveValueClamped01(const FName& CurveName) const
{
	return UBEMovementMath::Clamp01(GetCurveValue(CurveName));
}

#pragma endregion
