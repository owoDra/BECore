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
 *	Movement �̈ړ����
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
 * CharacterMovementComponent �ɒǉ��\�Ȉړ�����
 * CustomMovementMode �ɂĎg�p�����
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BECORE_API UBECharacterMovementFragment : public UObject
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) {}
#endif

protected:
	// ���� Movement �̈ړ����
	EBECustomMovementSpace MovementSpace = EBECustomMovementSpace::None;

	// ���� Movement �̎��ʗp Tag
	FGameplayTag MovementModeTag;

public:
	/**
	 * GetMovementSpace
	 *
	 * ���� Movement �̈ړ���Ԃ�Ԃ�
	 * CharacterMovementComponent �� IsMovingOnGround �ȂǂŔ��肷��̂ɗp����
	 */
	EBECustomMovementSpace GetMovementSpace() const { return MovementSpace; }

	/**
	 * GetMovementModeTag
	 *
	 * ���� Movement �̎��ʗp Tag ��Ԃ�
	 */
	FGameplayTag GetMovementModeTag() const { return MovementModeTag; }


public:
	/**
	 * GetMovementTypeTag
	 *
	 * ���� Movement �̎��ʗp Tag ��Ԃ�
	 */
	virtual bool CanStartInCurrentState(const UBECharacterMovementComponent* CMC) const { return false; }

	/**
	 * PhysMovement
	 *
	 * ���� Movement �̎��ۂ̏������s��
	 */
	virtual void PhysMovement(UBECharacterMovementComponent* CMC, float DeltaTime, int32 Iterations) {}

	/**
	 * TryStartMovement
	 *
	 * ���� Movement �̊J�n�����݂�
	 */
	virtual void TryStartMovement(UBECharacterMovementComponent* CMC) {}

	/**
	 * GetMaxMoveSpeed
	 *
	 * ���� Movement ���̍ō��ړ����x
	 */
	virtual float GetMaxMoveSpeed(const UBECharacterMovementComponent* CMC) const { return 0.0; }

	/**
	 * GetMaxBrakingDeceleration
	 *
	 * ���� Movement ���̍ō������x
	 */
	virtual float GetMaxBrakingDeceleration(const UBECharacterMovementComponent* CMC) const { return 0.0; }

	/**
	 * OnMovementStart
	 *
	 * ���� Movement �̏������J�n�����Ƃ��ɌĂяo�����
	 */
	virtual void OnMovementStart(const UBECharacterMovementComponent* CMC) {}

	/**
	 * OnMovementEnd
	 *
	 * ���� Movement �̏������I������Ƃ��ɌĂяo�����
	 */
	virtual void OnMovementEnd(const UBECharacterMovementComponent* CMC) {}

	/**
	 * OnLanded
	 *
	 * Character ���n�ʂɒ��n�����Ƃ��ɌĂяo�����
	 */
	virtual void OnLanded(const UBECharacterMovementComponent* CMC) {}
};
