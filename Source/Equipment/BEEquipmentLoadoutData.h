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
 * EquipmentManagerComponent に複数の Equipment を一度に追加する際に
 * 便利なデータアセット
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
