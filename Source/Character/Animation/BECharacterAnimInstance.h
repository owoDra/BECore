// Copyright owoDra

#pragma once

#include "Animation/BEAnimInstance.h"

#include "Character/Movement/State/BEControlRigInput.h"
#include "Character/Movement/State/BEFeetState.h"
#include "Character/Movement/State/BEGroundedState.h"
#include "Character/Movement/State/BEInAirState.h"
#include "Character/Movement/State/BELayeringState.h"
#include "Character/Movement/State/BELeanState.h"
#include "Character/Movement/State/BELocomotionAnimationState.h"
#include "Character/Movement/State/BEMovementBaseState.h"
#include "Character/Movement/State/BEPoseState.h"
#include "Character/Movement/State/BERotateInPlaceState.h"
#include "Character/Movement/State/BETransitionsState.h"
#include "Character/Movement/State/BEViewAnimationState.h"
#include "Character/Movement/State/BEViewState.h"
#include "Character/Movement/State/BELocomotionState.h"
#include "Character/Movement/State/BEMovementBaseState.h"

#include "BECharacterAnimInstance.generated.h"

class UBECharacterAnimData;
class UBECharacterLinkedAnimInstance;
class UBECharacterMovementComponent;
class ABECharacter;
enum class EHipsDirection : uint8;


/**
 * UBECharacterAnimInstance
 *
 *  BEAnimInstance から Character に対する機能に特化させた Mesh に適応するメインの AnimInsntace
 *	基本的に Character の TPP Mesh にのみ使用し Animation に必要なデータの処理を行う。
 */
UCLASS(Config = Game)
class BECORE_API UBECharacterAnimInstance : public UBEAnimInstance
{
	GENERATED_BODY()

	friend UBECharacterLinkedAnimInstance;

public:
	UBECharacterAnimInstance(const FObjectInitializer& ObjectInitializer);

protected:
	//
	// キャラクターのアニメーションに関する設定を定義したデータアセット
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs")
	TObjectPtr<UBECharacterAnimData> AnimData;

	//
	// この AnimInstance を所有している Mesh の Owner Character
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Refarence", Transient)
	TObjectPtr<ABECharacter> Character;

	//
	// Owner Character の Character Movement Component
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Refarence", Transient)
	TObjectPtr<UBECharacterMovementComponent> CharacterMovement;

protected:
	//
	// Character の現在の LocomotionMode
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag LocomotionMode;

	//
	// Character の現在の RotationMode
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag RotationMode;

	//
	// Character の現在の Stance
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Stance;

	//
	// Character の現在の Gait
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Gait;

	//
	// Character の現在の LocomotionAction
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag LocomotionAction;

	//
	// Character の現在の MovementBase
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FMovementBaseState MovementBase;

	//
	// Character の各部位の Animation のブレンドに用いる状態データ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FLayeringState LayeringState;

	//
	// Character の Animation のブレンドに用いるポーズのデータ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FPoseState PoseState;

	//
	// Character の視点データ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FViewAnimationState ViewState;

	//
	// Character の Lean Animation に用いるデータ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FLeanState LeanState;

	//
	// Character の移動全般のデータ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FLocomotionAnimationState LocomotionState;

	//
	// Character の地上にいる際に関係するデータ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGroundedState GroundedState;

	//
	// Character の空中にいる際に関係するデータ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FInAirState InAirState;

	//
	// Character の足の配置などに関するデータ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FFeetState FeetState;

	//
	// Character の状態遷移 Animation に関するデータ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FTransitionsState TransitionsState;

	//
	// Character のその場回転 Animation に関するデータ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotateInPlaceState RotateInPlaceState;


public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;
	virtual void NativePostEvaluateAnimation() override;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;


	/////////////////////////////////////////
	// Core
protected:
	//
	// アニメーションのインスタンスが長い間更新されておらず
	// 現在の状態が正しくない可能性があることを示しますフラグ
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	bool bPendingUpdate = true;

	//
	// 最後にテレポートが行われた時間
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0))
	float TeleportedTime;

protected:
	/**
	 * GetAnimDataUnsafe
	 * 
	 *  BlueprintThreadSafe で AnimData を取得する
	 */
	UFUNCTION(BlueprintPure, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe, ReturnDisplayName = "Setting"))
	UBECharacterAnimData* GetAnimDataUnsafe() const { return AnimData; }

	/**
	 * GetAnimDataUnsafe
	 *
	 *  BlueprintThreadSafe で ControlRig へ渡すためのデータを取得する
	 */
	UFUNCTION(BlueprintPure, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe, ReturnDisplayName = "Rig Input"))
	FControlRigInput GetControlRigInput() const;

public:
	void MarkPendingUpdate() { bPendingUpdate |= true; }

	void MarkTeleported();

private:
	void UpdateMovementBaseOnGameThread();

	void UpdateLayering();

	void UpdatePose();


	/////////////////////////////////////////
	// View
public:
	virtual bool IsSpineRotationAllowed();

private:
	void UpdateViewOnGameThread();

	void UpdateView(float DeltaTime);

	void UpdateSpineRotation(float DeltaTime);

protected:
	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ReinitializeLook();

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void UpdateLook();


	/////////////////////////////////////////
	// Locomotion
private:
	void UpdateLocomotionOnGameThread();


	/////////////////////////////////////////
	// OnGround
protected:
	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void SetHipsDirection(EHipsDirection NewHipsDirection);

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ActivatePivot();

private:
	void UpdateGroundedOnGameThread();

	void UpdateGrounded(float DeltaTime);

	void UpdateMovementDirection();

	void UpdateVelocityBlend(float DeltaTime);

	void UpdateRotationYawOffsets();

	void UpdateSprint(const FVector3f& RelativeAccelerationAmount, float DeltaTime);

	void UpdateStrideBlendAmount();

	void UpdateWalkRunBlendAmount();

	void UpdateStandingPlayRate();

	void UpdateCrouchingPlayRate();

	void UpdateGroundedLeanAmount(const FVector3f& RelativeAccelerationAmount, float DeltaTime);

	void ResetGroundedLeanAmount(float DeltaTime);


	/////////////////////////////////////////
	// InAir
public:
	void Jump();

protected:
	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ResetJumped();

private:
	void UpdateInAirOnGameThread();

	void UpdateInAir(float DeltaTime);

	void UpdateGroundPredictionAmount();

	void UpdateInAirLeanAmount(float DeltaTime);


	/////////////////////////////////////////
	// Feet
private:
	void UpdateFeetOnGameThread();

	void UpdateFeet(float DeltaTime);

	void UpdateFoot(FFootState& FootState, const FName& FootIkCurveName, const FName& FootLockCurveName, const FTransform& ComponentTransformInverse, float DeltaTime) const;

	void ProcessFootLockTeleport(FFootState& FootState) const;

	void ProcessFootLockBaseChange(FFootState& FootState, const FTransform& ComponentTransformInverse) const;

	void UpdateFootLock(FFootState& FootState, const FName& FootLockCurveName, const FTransform& ComponentTransformInverse, float DeltaTime, FVector& FinalLocation, FQuat& FinalRotation) const;

	void UpdateFootOffset(FFootState& FootState, float DeltaTime, FVector& FinalLocation, FQuat& FinalRotation) const;


	/////////////////////////////////////////
	// Transitions
public:
	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void PlayQuickStopAnimation();

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void PlayTransitionAnimation(UAnimSequenceBase* Animation, float BlendInDuration = 0.2f, float BlendOutDuration = 0.2f, float PlayRate = 1.0f, float StartTime = 0.0f, bool bFromStandingIdleOnly = false);

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void PlayTransitionLeftAnimation(float BlendInDuration = 0.2f, float BlendOutDuration = 0.2f, float PlayRate = 1.0f, float StartTime = 0.0f, bool bFromStandingIdleOnly = false);

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void PlayTransitionRightAnimation(float BlendInDuration = 0.2f, float BlendOutDuration = 0.2f, float PlayRate = 1.0f, float StartTime = 0.0f, bool bFromStandingIdleOnly = false);

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void StopTransitionAndTurnInPlaceAnimations(float BlendOutDuration = 0.2f);

private:
	void UpdateTransitions();

	void UpdateDynamicTransition();

	void PlayQueuedDynamicTransitionAnimation();


	/////////////////////////////////////////
	// Rotate In Place
public:
	virtual bool IsRotateInPlaceAllowed();

private:
	void UpdateRotateInPlace(float DeltaTime);


	/////////////////////////////////////////
	// Utilities
public:
	float GetCurveValueClamped01(const FName& CurveName) const;

};
