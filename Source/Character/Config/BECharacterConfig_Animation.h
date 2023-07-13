// Copyright owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "BECharacterConfig_Animation.generated.h"

class UCurveFloat;
class UAnimSequenceBase;


/**
 * FAnimGeneralConfigs
 *
 *  基本的なアニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct BECORE_API FAnimGeneralConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	bool bUseHandIkBones = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	bool bUseFootIkBones = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSmoothSpeedThreshold = 150.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0))
	float LeanInterpolationSpeed = 4.0;
};


/**
 * FAnimFeetConfigs
 *
 *  足のIKなどに関係するアニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct BECORE_API FAnimFeetConfigs
{
	GENERATED_BODY()
public:
	FAnimFeetConfigs();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	bool bDisableFootLock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float FootHeight = 13.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TEnumAsByte<ETraceTypeQuery> IkTraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float IkTraceDistanceUpward = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float IkTraceDistanceDownward = 45.0;
};


/**
 * FAnimViewConfigs
 * 
 *  視点によるアニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct BECORE_API FAnimViewConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0))
	float LookTowardsCameraRotationInterpolationSpeed = 8.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0))
	float LookTowardsInputYawAngleInterpolationSpeed = 8.0;
};


/**
 * FAnimGroundedConfigs
 *
 *  地上にいる時のアニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct BECORE_API FAnimGroundedConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float AnimatedWalkSpeed = 150.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float AnimatedRunSpeed = 350.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float AnimatedSprintSpeed = 600.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float AnimatedCrouchSpeed = 150.0;

	// 
	// 移動速度とストライドのブレンド量カーブ
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UCurveFloat> StrideBlendAmountWalkCurve = nullptr;

	// 
	// 移動速度とストライドのブレンド量カーブ
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UCurveFloat> StrideBlendAmountRunCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UCurveFloat> RotationYawOffsetForwardCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UCurveFloat> RotationYawOffsetBackwardCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UCurveFloat> RotationYawOffsetLeftCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UCurveFloat> RotationYawOffsetRightCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0))
	float VelocityBlendInterpolationSpeed = 12.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float PivotActivationSpeedThreshold = 200;
};


/**
 * FAnimInAirConfigs
 *
 *  地上にいる時のアニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct BECORE_API FAnimInAirConfigs
{
	GENERATED_BODY()
public:
	FAnimInAirConfigs();

#if WITH_EDITOR
	void PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent);
#endif

public:
	//
	// 落下速度と Lean 量のカーブ
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UCurveFloat> LeanAmountCurve = nullptr;

	//
	// 地面予測スイープヒット時間から地面予測量のカーブ
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UCurveFloat> GroundPredictionAmountCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TArray<TEnumAsByte<EObjectTypeQuery>> GroundPredictionSweepObjectTypes;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Configs")
	FCollisionResponseContainer GroundPredictionSweepResponses;
};


/**
 * FAnimRotateInPlaceConfigs
 *
 *  その場回転のアニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct BECORE_API FAnimRotateInPlaceConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float ViewYawAngleThreshold = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0))
	FVector2f ReferenceViewYawSpeed = { 180.0, 460.0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0))
	FVector2f PlayRate { 1.15, 3.0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	bool bDisableFootLock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ClampMax = 180, EditCondition = "!bDisableFootLock", ForceUnits = "deg"))
	float FootLockBlockViewYawAngleThreshold = 120.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, EditCondition = "!bDisableFootLock", ForceUnits = "deg/s"))
	float FootLockBlockViewYawSpeedThreshold = 620.0;
};


/**
 * UAnimTurnInPlaceConfigs
 *
 *  その場回転のアニメーションの設定を定義する
 */
UCLASS(BlueprintType, EditInlineNew)
class BECORE_API UAnimTurnInPlaceConfigs : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "x"))
	float PlayRate = 1.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	bool bScalePlayRateByAnimatedTurnAngle = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float AnimatedTurnAngle;
};


/**
 * FAnimGeneralTurnInPlaceConfigs
 *
 *  その場回転のアニメーションの汎用的な設定を定義する
 */
USTRUCT(BlueprintType)
struct BECORE_API FAnimGeneralTurnInPlaceConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float ViewYawAngleThreshold = 45.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float ViewYawSpeedThreshold = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0))
	FVector2f ViewYawAngleToActivationDelay = { 0.0, 0.75 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", DisplayName = "Turn 180 Angle Threshold", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float Turn180AngleThreshold = 130.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "s"))
	float BlendDuration = 0.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	bool bDisableFootLock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced, DisplayName = "Standing Turn 90 Left")
	TObjectPtr<UAnimTurnInPlaceConfigs> StandingTurn90Left = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced, DisplayName = "Standing Turn 90 Right")
	TObjectPtr<UAnimTurnInPlaceConfigs> StandingTurn90Right = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced, DisplayName = "Standing Turn 180 Left")
	TObjectPtr<UAnimTurnInPlaceConfigs> StandingTurn180Left = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced, DisplayName = "Standing Turn 180 Right")
	TObjectPtr<UAnimTurnInPlaceConfigs> StandingTurn180Right = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced, DisplayName = "Crouching Turn 90 Left")
	TObjectPtr<UAnimTurnInPlaceConfigs> CrouchingTurn90Left = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced, DisplayName = "Crouching Turn 90 Right")
	TObjectPtr<UAnimTurnInPlaceConfigs> CrouchingTurn90Right = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced, DisplayName = "Crouching Turn 180 Left")
	TObjectPtr<UAnimTurnInPlaceConfigs> CrouchingTurn180Left = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced, DisplayName = "Crouching Turn 180 Right")
	TObjectPtr<UAnimTurnInPlaceConfigs> CrouchingTurn180Right = nullptr;
};


/**
 * FAnimTransitionsConfigs
 *
 *  遷移アニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct BECORE_API FAnimTransitionsConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "s"))
	float QuickStopBlendInDuration = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "s"))
	float QuickStopBlendOutDuration = 0.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0))
	FVector2f QuickStopPlayRate = { 1.75f, 3.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "s"))
	float QuickStopStartTime = 0.3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UAnimSequenceBase> StandingTransitionLeftAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UAnimSequenceBase> StandingTransitionRightAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UAnimSequenceBase> CrouchingTransitionLeftAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UAnimSequenceBase> CrouchingTransitionRightAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float DynamicTransitionFootLockDistanceThreshold = 8.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "s"))
	float DynamicTransitionBlendDuration = 0.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "x"))
	float DynamicTransitionPlayRate = 1.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UAnimSequenceBase> StandingDynamicTransitionLeftAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UAnimSequenceBase> StandingDynamicTransitionRightAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UAnimSequenceBase> CrouchingDynamicTransitionLeftAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TObjectPtr<UAnimSequenceBase> CrouchingDynamicTransitionRightAnimation = nullptr;
};


/**
 * UBECharacterAnimConfigs
 *
 *  キャラクターのアニメーションの設定を定義する
 */
UCLASS(Blueprintable, BlueprintType)
class BECORE_API UBECharacterAnimConfigs : public UDataAsset
{
	GENERATED_BODY()
public:
	UBECharacterAnimConfigs();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimGeneralConfigs General;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimViewConfigs View;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimGroundedConfigs Grounded;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimInAirConfigs InAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimFeetConfigs Feet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimTransitionsConfigs Transitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimRotateInPlaceConfigs RotateInPlace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimGeneralTurnInPlaceConfigs TurnInPlace;
};
