// Copyright owoDra

#pragma once

#include "BECharacterMovementConfigs.generated.h"

class UBECharacterMovementComponent;
class UBECharacterMovementCondition;
struct FGameplayTag;


/**
 * FBECharacterGaitConfigs
 * 
 *  キャラクターの Gait ごとの設定を定義する
 */
USTRUCT(BlueprintType)
struct FBECharacterGaitConfigs
{
	GENERATED_BODY()
public:
	FBECharacterGaitConfigs() {}

	FBECharacterGaitConfigs(const FGameplayTag& InGaitTag, float InMaxSpeed, float InMaxAcceleration, float InBrakingDeceleration,
		float InGroundFriction, float InJumpZPower, float InAirControl, float InRotationInterpSpeed);

public:
	//
	// この Gait を識別するための GameplayTag
	// この Gait が適応されたときに Ability System に登録される
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (Categories = "Status.Gait"))
	FGameplayTag GaitTag;

	//
	// この Gait が適応中の最大移動速度
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MaxSpeed;

	//
	// この Gait が適応中の最大加速度
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ForceUnits = "cm/s"))
	float MaxAcceleration;

	//
	// この Gait が適応中の減速度
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (ForceUnits = "cm/s"))
	float BrakingDeceleration;

	//
	// この Gait が適応中の摩擦係数
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	float GroundFriction;

	//
	// この Gait が適応中のジャンプ力
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	float JumpZPower;

	//
	// この Gait が適応中の空中制御性能
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	float AirControl;

	//
	// この Gait が適応中の回転補間速度
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	float RotationInterpSpeed;

	//
	// 遷移可能かどうかの条件
	// 設定しない場合は無条件で遷移可能
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced)
	TObjectPtr<UBECharacterMovementCondition> EnterCondition = nullptr;
};


/**
 * FBECharacterStanceConfigs
 *
 *  キャラクターの Stance ごとの設定を定義する
 */
USTRUCT(BlueprintType)
struct FBECharacterStanceConfigs
{
	GENERATED_BODY()
public:
	FBECharacterStanceConfigs() {}

	FBECharacterStanceConfigs(const FGameplayTag& InStanceTag);

public:
	//
	// この Stance を識別するための GameplayTag
	// この Stance が適応されたときに Ability System に登録される
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (Categories = "Status.Stance"))
	FGameplayTag StanceTag;

	//
	// 遷移可能かどうかの条件
	// 設定しない場合は無条件で遷移可能
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced)
	TObjectPtr<UBECharacterMovementCondition> EnterCondition = nullptr;

	//
	// この Stance 適応中に許可されている Gait
	//
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configs")
	TArray<FBECharacterGaitConfigs> Gaits;
};


/**
 * FBECharacterRotationModeConfigs
 *
 *  キャラクターの RotationMode ごとの設定を定義する
 */
USTRUCT(BlueprintType)
struct FBECharacterRotationModeConfigs
{
	GENERATED_BODY()
public:
	FBECharacterRotationModeConfigs() {}

	FBECharacterRotationModeConfigs(const FGameplayTag& InRotationModeTag);

public:
	//
	// この RotationMode を識別するための GameplayTag
	// この RotationMode が適応されたときに Ability System に登録される
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (Categories = "Status.RotationMode"))
	FGameplayTag RotationModeTag;

	//
	// 遷移可能かどうかの条件
	// 設定しない場合は無条件で遷移可能
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced)
	TObjectPtr<UBECharacterMovementCondition> EnterCondition = nullptr;

	//
	// この RotationMode 適応中に許可されている Stance
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TArray<FBECharacterStanceConfigs> Stances;
};


/**
 * FBECharacterLocomotionModeConfigs
 *
 *  キャラクターの LocomotionMode ごとの設定を定義する
 */
USTRUCT(BlueprintType)
struct FBECharacterLocomotionModeConfigs
{
	GENERATED_BODY()
public:
	FBECharacterLocomotionModeConfigs();

public:
	//
	// この RotationMode 適応中に許可されている Stance
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs")
	TArray<FBECharacterRotationModeConfigs> RotationModes;
};
