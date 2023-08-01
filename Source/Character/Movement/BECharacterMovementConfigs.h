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
	FBECharacterGaitConfigs();

	FBECharacterGaitConfigs(float InMaxSpeed, float InMaxAcceleration, float InBrakingDeceleration, float InGroundFriction, float InJumpZPower, float InAirControl, float InRotationInterpSpeed, const TObjectPtr<UBECharacterMovementCondition>& InCondition);

public:
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
	// 遷移可能かどうかの条件を定義する
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced)
	TObjectPtr<UBECharacterMovementCondition> Condition;
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
	FBECharacterStanceConfigs();

public:
	//
	// 無効な Gait が設定された際に遷移する Gait
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (Categories = "Status.Gait"))
	FGameplayTag DefaultGait;

	//
	// この Stance 適応中に許可されている Gait
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (Categories = "Status.Gait", ForceInlineRow))
	TMap<FGameplayTag, FBECharacterGaitConfigs> Gaits;

public:
	/**
	 * GetAllowedGait
	 *
	 *  遷移可能な GaitTag を取得する
	 */
	const FBECharacterGaitConfigs& GetAllowedGait(const UBECharacterMovementComponent* CMC, const FGameplayTag& DesiredGait, FGameplayTag& OutTag) const;
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
	FBECharacterRotationModeConfigs();

public:
	//
	// この RotationMode 適応中のデフォルトの Stance
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (Categories = "Status.Stance"))
	FGameplayTag DefaultStance;

	//
	// この RotationMode 適応中に許可されている Stance
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (Categories = "Status.Stance", ForceInlineRow))
	TMap<FGameplayTag, FBECharacterStanceConfigs> Stances;

	//
	// 遷移可能かどうかの条件を定義する
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Instanced)
	TObjectPtr<UBECharacterMovementCondition> Condition;

public:
	/**
	 * GetAllowedStance
	 *
	 *  遷移可能な StanceTag を取得する
	 */
	const FBECharacterStanceConfigs& GetAllowedStance(const FGameplayTag& DesiredStance, FGameplayTag& OutTag) const;
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
	// この Locomotion 適応中のデフォルトの RotationMode
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (Categories = "Status.RotationMode"))
	FGameplayTag DefaultRotationMode;

	//
	// この Locomotion 適応中に許可されている RotationMode
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configs", Meta = (Categories = "Status.RotationMode", ForceInlineRow))
	TMap<FGameplayTag, FBECharacterRotationModeConfigs> RotationModes;

public:
	/**
	 * GetAllowedRotationMode
	 *
	 *  遷移可能な RotationModeTag を取得する
	 */
	const FBECharacterRotationModeConfigs& GetAllowedRotationMode(const UBECharacterMovementComponent* CMC, const FGameplayTag& DesiredRotationMode, FGameplayTag& OutTag) const;
};
