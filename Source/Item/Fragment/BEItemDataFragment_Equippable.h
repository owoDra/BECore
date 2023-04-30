// Copyright Eigi Chin

#pragma once

#include "Item/BEItemData.h"

#include "Containers/Array.h"
#include "Math/Transform.h"
#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEItemDataFragment_Equippable.generated.h"

class UBEAbilitySet;
class UBEEquipmentInstance;
struct FGameplayTagContainer;


/**
 * UBEItemDataFragment_Equippable
 *
 * Item �������\���ǂ��������ʂ��邽�߂Ɏg�p�B
 * Item �𑕔������Ƃ��̏�񂪊܂܂��B
 */
UCLASS()
class UBEItemDataFragment_Equippable : public UBEItemDataFragment
{
	GENERATED_BODY()

public:
	// �����w�肵�Ȃ��ꍇ�͑S�ẴX���b�g�Ɋi�[�\
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (Categories = "Equipment.Slot"))
	FGameplayTagContainer AllowedSlotTags;

	// �����i�̐��\���Ԃ��Ǘ����邽�߂̃N���X
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UBEEquipmentInstance> InstanceType;

	// �������ɑ�������Pawn�ɕt�^����A�r���e�B�Z�b�g
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TArray<TObjectPtr<const UBEAbilitySet>> AbilitySetsToGrantOnEquip;

	// �A�N�e�B�u���ɑ�������Pawn�ɕt�^����A�r���e�B�Z�b�g
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TArray<TObjectPtr<const UBEAbilitySet>> AbilitySetsToGrantOnActive;
};
