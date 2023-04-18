// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "Containers/Map.h"
#include "HAL/Platform.h"
#include "Internationalization/Text.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEUserFacingPawnData.generated.h"

class UTexture2D;
class FString;
class UObject;
class UTexture2D;
struct FFrame;


//======================================
//	構造体
//======================================

/**
 * FBESubclassedPawnData
 * 
 * キャラクターのサブクラスに対応したPawnDataを定義する
 */
USTRUCT(BlueprintType)
struct FBESubclassedPawnData
{
	GENERATED_BODY()

public:

	// PawnDataのプライマリーアセットID
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData, meta = (AllowedTypes = "BEPawnData"))
		FPrimaryAssetId PawnData;

	// サブクラスのタイトル
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		FText Title;

	// サブクラスの説明
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		FText Description;

	// サブクラスのアイコン
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		TObjectPtr<UTexture2D> Icon;

	// デフォルトのPawnDataなのか
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
 * UBEUserFacingPawnData
 *
 *	キャラクター選択メニューなどに用いるデータアセット
 */
UCLASS(BlueprintType)
class UBEUserFacingPawnData : public UPrimaryDataAsset
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
		TArray<FBESubclassedPawnData> SubclassPawnDatas;

	// UIなどに表示するかどうか
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PawnData)
		bool bShowInFrontEnd = true;
};
