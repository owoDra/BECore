// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BECharacterData.generated.h"

class APawn;
class UBEAbilitySet;
class UBEInputConfig;
class UBEAbilityTagRelationshipMapping;
class UBECameraMode;
class UBEItemData;
class UObject;
struct FGameplayTag;


/**
 * UBECharacterData
 *
 *	Character の基本情報を定義する
 */
UCLASS(BlueprintType, Const)
class UBECharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	UBECharacterData(const FObjectInitializer& ObjectInitializer);

public:
	// スポーンする Pawn のクラス (通常、ABEPawn または ABECharacter から派生する必要があります)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSubclassOf<APawn> PawnClass;

	// この Pawn の AbilitySystem に付与する AbilitySet 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UBEAbilitySet>> AbilitySets;

	// この Pawn が実行する GameplayAbility に使用する Tag のマッピング
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UBEAbilityTagRelationshipMapping> TagRelationshipMapping;

	// 入力マッピングを作成し、入力アクションをバインドするために、プレーヤーが制御する Pawn によって使用される入力構成
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UBEInputConfig> InputConfig;

	// プレイヤーが制御する Pawn が使用するデフォルトの CameraMode
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<UBECameraMode> DefaultCameraMode;

	// プレイヤーが制御する Pawn に適応されるデフォルトの Equipments
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	TMap<FGameplayTag, TObjectPtr<UBEItemData>> DefaultEquipments;

	// プレイヤーが制御する Pawn に適応されるデフォルトの Active は Slot
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	FGameplayTag DefaultActiveSlotTag;
};
