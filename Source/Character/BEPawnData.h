// Copyright owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BEPawnData.generated.h"

class APawn;
class UBEAbilitySet;
class UBEInputConfig;
class UBEAbilityTagRelationshipMapping;
class UBECameraMode;
class UBEEquipmentSet;
class UObject;


/**
 * UBEPawnData
 *
 *	Character の基本情報を定義する
 */
UCLASS(BlueprintType, Const)
class UBEPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBEPawnData(const FObjectInitializer& ObjectInitializer);

public:
	// スポーンする Character のクラス (通常、ABEPawn または ABECharacter から派生する必要があります)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSubclassOf<APawn> PawnClass;

	// この Character の AbilitySystem に付与する AbilitySet 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UBEAbilitySet>> AbilitySets;

	// この Character が実行する GameplayAbility に使用する Tag のマッピング
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UBEAbilityTagRelationshipMapping> TagRelationshipMapping;

	// 入力マッピングを作成し、入力アクションをバインドするために、プレーヤーが制御する Pawn によって使用される入力構成
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UBEInputConfig> InputConfig;

	// プレイヤーが制御する Character が使用するデフォルトの CameraMode
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<UBECameraMode> DefaultCameraMode;

	// デフォルトで装備している Equipments
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UBEEquipmentSet> EquipmentSet;

};
