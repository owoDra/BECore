// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "BECharacterEquipmentComponent.h"

#include "Internationalization/Text.h"
#include "UObject/UObjectGlobals.h"

#include "BEEquipmentLoadoutData.generated.h"


/**
 * UBEEquipmentLoadoutData
 * 
 * EquipmentManagerComponent �ɕ����� Equipment ����x�ɒǉ�����ۂ�
 * �֗��ȃf�[�^�A�Z�b�g
 */
UCLASS(BlueprintType, Const, Abstract)
class UBEEquipmentLoadoutData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBEEquipmentLoadoutData();
	
#if WITH_EDITOR //~UObject interface
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif //~End of UObject interface

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FBEEquipmentLoadout Loadout;
};
