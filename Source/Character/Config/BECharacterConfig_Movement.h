// Copyright owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "BECharacterConfig_Movement.generated.h"

class UCurveFloat;
class UCurveVector;
class UAnimMontage;
class UBECharacterMovementFragment;
struct FGameplayTag;


/**
 * FViewConfigs
 *
 *  キャラクターの視点に関する設定を定義する
 */
USTRUCT(BlueprintType)
struct FViewConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs")
	bool bEnableNetworkSmoothing = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs")
	bool bEnableListenServerNetworkSmoothing = true;
};


/**
 * FMovementGaitConfigs
 * 
 *  キャラクターの Gait ごとの設定を定義する
 */
USTRUCT(BlueprintType)
struct FMovementGaitConfigs
{
	GENERATED_BODY()
public:
	FMovementGaitConfigs(
		float InMaxSpeed,
		float InMaxAcceleration,
		float InMaxDeceleration,
		float InGroundFriction,
		float InRotationInterpSpeed)
		: MaxSpeed(InMaxSpeed)
		, MaxAcceleration(InMaxAcceleration)
		, MaxDeceleration(InMaxDeceleration)
		, GroundFriction(InGroundFriction)
		, RotationInterpSpeed(InRotationInterpSpeed)
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ForceUnits = "cm/s"))
	float MaxAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ForceUnits = "cm/s"))
	float MaxDeceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	float GroundFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	float RotationInterpSpeed;
};


/**
 * FMovementStanceConfigs
 *
 *  キャラクターの Stance ごとの設定を定義する
 */
USTRUCT(BlueprintType)
struct FMovementStanceConfigs
{
	GENERATED_BODY()
public:
	FMovementStanceConfigs();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ForceInlineRow, Categories = "Status.Gait"))
	TMap<FGameplayTag, FMovementGaitConfigs> Gaits;
};


/**
 * FMovementLocomotionModeConfigs
 *
 *  キャラクターの LocomotionMode ごとの設定を定義する
 */
USTRUCT(BlueprintType)
struct FMovementLocomotionModeConfigs
{
	GENERATED_BODY()
public:
	FMovementLocomotionModeConfigs();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ForceInlineRow))
	TMap<FGameplayTag, FMovementStanceConfigs> RotationModes;
};


/**
 * UBECharacterMovementConfigs
 *
 *  キャラクターの移動設定を定義する
 */
UCLASS(Blueprintable, BlueprintType)
class BECORE_API UBECharacterMovementConfigs : public UDataAsset
{
	GENERATED_BODY()
public:
	UMovementConfigs();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	//
	// State ごとの移動速度などの設定
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs", Meta = (ForceInlineRow))
	TMap<FGameplayTag, FMovementLocomotionModeConfigs> LocomotionModes;

	//
	// CustomMovementMode の Movement 処理
	// 配列の順番が CustomMovement の番号と一致する
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs", Instanced)
	TArray<TObjectPtr<UBECharacterMovementFragment>> Fragments;

	//
	// 移動中であると判定される速度の閾値
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSpeedThreshold = 50.0;

	// 有効にすると、キャラクターは立っているオブジェクトに対して速度で回転します。
	// 方向回転モード。それ以外の場合、キャラクターはそのオブジェクトを無視し、ワールド回転を維持します。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs")
	bool bInheritMovementBaseRotationInVelocityDirectionRotationMode;

	// 有効にすると、キャラクターは移動したい方向に回転しますが、障害物があるため常に回転できるとは限りません。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs")
	bool bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs")
	FViewConfigs View;
};
