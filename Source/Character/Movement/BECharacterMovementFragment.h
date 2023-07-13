// Copyright owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "UObject/UObjectGlobals.h"

#include "BECharacterMovementFragment.generated.h"

class UBECharacterMovementComponent;
struct FGameplayTag;
struct FGameplayTagContainer;


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
	//
	// この Movement の LocomotionMode の Tag
	//
	FGameplayTag LocomotionModeTag;

public:
	/**
	 * GetLocomotionMode
	 *
	 * この Movement の LocomotionMode を返す
	 */
	const FGameplayTag& GetLocomotionMode() const { return LocomotionModeTag; }

	/**
	 * CalculateAllowedStance
	 *
	 * 現在の Movement で許可されている Stance を返す
	 */
	virtual FGameplayTag CalculateAllowedStance(const FGameplayTag& DesiredStance) const { return DesiredStance; }

	/**
	 * CalculateAllowedGait
	 *
	 * 現在の Movement で許可されている Gait を返す
	 */
	virtual FGameplayTag CalculateAllowedGait(const FGameplayTag& DesiredGait) const { return DesiredGait; }

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
