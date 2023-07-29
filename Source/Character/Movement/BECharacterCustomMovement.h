// Copyright owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "UObject/UObjectGlobals.h"

#include "BECharacterCustomMovement.generated.h"

class UBECharacterMovementComponent;
struct FGameplayTag;
struct FGameplayTagContainer;


/**
 * UBECharacterCustomMovement
 * 
 * CharacterMovementComponent に追加可能な移動処理
 * CustomMovementMode にて使用される
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BECORE_API UBECharacterCustomMovement : public UObject
{
	GENERATED_BODY()
public:

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

public:
	/**
	 * PhysMovement
	 *
	 * この Movement の実際の処理を行う
	 */
	virtual void PhysMovement(UBECharacterMovementComponent* CMC, float DeltaTime, int32 Iterations) {}

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
};
