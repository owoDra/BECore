// Copyright owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "BECharacterMovementConfigs.h"

#include "UObject/ObjectPtr.h"
#include "Containers/Array.h"

#include "BECharacterMovementData.generated.h"

class UCurveFloat;
class UCurveVector;
class UAnimMontage;
class UBECharacterCustomMovement;
struct FGameplayTag;


/**
 * UBECharacterMovementData
 *
 *  キャラクターの移動設定を定義する
 */
UCLASS(Blueprintable, BlueprintType)
class BECORE_API UBECharacterMovementData : public UDataAsset
{
	GENERATED_BODY()
public:
	UBECharacterMovementData();

public:
	//
	// LocomotionModes ごとの基本的な移動設定
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", Meta = (ForceInlineRow))
	TMap<FGameplayTag, FBECharacterLocomotionModeConfigs> LocomotionModes;

	//
	// CustomMovementMode の Movement 処理
	// 配列の順番が CustomMovement の番号と一致する
	// CustomMovement の LocomotionMode 中の基本的な移動設定は
	// LocomotionModes で定義することができます。
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", Instanced)
	TArray<TObjectPtr<UBECharacterCustomMovement>> CustomMovements;

	//
	// 移動中であると判定される速度の閾値
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSpeedThreshold = 50.0;

	//
	// 有効にすると、キャラクターは立っているオブジェクトに対して速度で回転します。
	// 方向回転モード。それ以外の場合、キャラクターはそのオブジェクトを無視し、ワールド回転を維持します。
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	bool bInheritMovementBaseRotationInVelocityDirectionRotationMode = true;

	//
	// 有効にすると、キャラクターは移動したい方向に回転しますが、障害物があるため常に回転できるとは限りません。
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	bool bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode = true;

	//
	// ネットスムージングを使用するかどうか
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Netowork")
	bool bEnableNetworkSmoothing = true;

	//
	// サーバーからのネットスムージングをリッスンするかどうか
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Netowork")
	bool bEnableListenServerNetworkSmoothing = true;
};
