// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "Containers/Map.h"
#include "HAL/Platform.h"
#include "Internationalization/Text.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEUserFacingEquipmentItemDefinition.generated.h"

class UTexture2D;
class FString;
class UObject;
class UTexture2D;
struct FFrame;


//======================================
//	構造体
//======================================

/**
 * FBESkinEquipmentItem
 *
 * 装備のスキンのItem Definitionなどを定義
 */
USTRUCT(BlueprintType)
struct FBESkinEquipmentItem
{
	GENERATED_BODY()

public:

	// Item DefinitionのプライマリーアセットID
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData, meta = (AllowedTypes = "BEInventoryItemDefinition"))
		FPrimaryAssetId ItemDef;

	// タイトル
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		FText Title;

	// 説明
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		FText Description;

	// アイコン
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		TObjectPtr<UTexture2D> Icon;

	// デフォルトのスキンなのか
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		bool bIsDefaultPawnData = false;

	// オプション。たとえばアンロック状況を参照するためのキーなど
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		TMap<FString, FString> ExtraArgs;
};


//======================================
//	クラス
//======================================

/**
 * UBEUserFacingEquipmentItemDefinition
 *
 *	装備の選択に用いるデータアセット
 */
UCLASS(BlueprintType)
class UBEUserFacingEquipmentItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// クラスのタイトル
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		FText Title;

	// クラスの説明
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		FText Description;

	// クラスのアイコン
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		UTexture2D* Icon;

	// サブクラス
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		TArray<FBESkinEquipmentItem> SkinEquipmentItems;

	// UIなどに表示するかどうか
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		bool bShowInFrontEnd = true;
};
