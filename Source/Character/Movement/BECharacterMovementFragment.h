// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "UObject/UObjectGlobals.h"
#include "NativeGameplayTags.h"

#include "BECharacterMovementFragment.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_CustomMode);

class UBECharacterMovementComponent;
struct FGameplayTag;


/**
 * EBECustomMovementSpace
 *
 *	Movement の移動空間
 */
UENUM(BlueprintType)
enum class EBECustomMovementSpace : uint8
{
	None,
	OnGround,
	InAir,
	InWater
};


/**
 * UBECharacterMovementFragment
 * 
 * CharacterMovementComponent に追加可能な移動処理
 * CustomMovementMode にて使用される
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BECORE_API UBECharacterMovementFragment : public UObject
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) {}
#endif

protected:
	// この Movement の移動空間
	EBECustomMovementSpace MovementSpace = EBECustomMovementSpace::None;

	// この Movement の識別用 Tag
	FGameplayTag MovementModeTag;

public:
	/**
	 * GetMovementSpace
	 *
	 * この Movement の移動空間を返す
	 * CharacterMovementComponent で IsMovingOnGround などで判定するのに用いる
	 */
	EBECustomMovementSpace GetMovementSpace() const { return MovementSpace; }

	/**
	 * GetMovementModeTag
	 *
	 * この Movement の識別用 Tag を返す
	 */
	FGameplayTag GetMovementModeTag() const { return MovementModeTag; }


public:
	/**
	 * GetMovementTypeTag
	 *
	 * この Movement の識別用 Tag を返す
	 */
	virtual bool CanStartInCurrentState(const UBECharacterMovementComponent* CMC) const { return false; }

	/**
	 * PhysMovement
	 *
	 * この Movement の実際の処理を行う
	 */
	virtual void PhysMovement(UBECharacterMovementComponent* CMC, float DeltaTime, int32 Iterations) {}

	/**
	 * TryStartMovement
	 *
	 * この Movement の開始を試みる
	 */
	virtual void TryStartMovement(UBECharacterMovementComponent* CMC) {}

	/**
	 * GetMaxMoveSpeed
	 *
	 * この Movement 中の最高移動速度
	 */
	virtual float GetMaxMoveSpeed(const UBECharacterMovementComponent* CMC) const { return 0.0; }

	/**
	 * GetMaxBrakingDeceleration
	 *
	 * この Movement 中の最高減速度
	 */
	virtual float GetMaxBrakingDeceleration(const UBECharacterMovementComponent* CMC) const { return 0.0; }

	/**
	 * OnMovementStart
	 *
	 * この Movement の処理が開始したときに呼び出される
	 */
	virtual void OnMovementStart(const UBECharacterMovementComponent* CMC) {}

	/**
	 * OnMovementEnd
	 *
	 * この Movement の処理が終了するときに呼び出される
	 */
	virtual void OnMovementEnd(const UBECharacterMovementComponent* CMC) {}

	/**
	 * OnLanded
	 *
	 * Character が地面に着地したときに呼び出される
	 */
	virtual void OnLanded(const UBECharacterMovementComponent* CMC) {}
};
