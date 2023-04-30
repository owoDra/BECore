// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "Containers/Array.h"
#include "Internationalization/Text.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEItemData.generated.h"


/**
 * UBEItemDataFragment
 * 
 * Item �ɕt�^�\�Ȓǉ����
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BECORE_API UBEItemDataFragment : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) {}
#endif
};


/**
 * UBEItemData
 * 
 * Item ���`����B
 * Item �̖��O��ڍׂȂǂ̂��炩���ߌ��߂�ꂽ���Ȃǂ�ۗL����B
 */
UCLASS(BlueprintType, Const, Abstract)
class UBEItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBEItemData();
	
#if WITH_EDITOR //~UObject interface
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif //~End of UObject interface
	
#if WITH_EDITORONLY_DATA //~UPrimaryDataAsset interface
	virtual void UpdateAssetBundleData() override;
#endif //~End of UPrimaryDataAsset interface


public:
	// �A�C�e���̃Q�[�����ł̕\����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category= "Inventory", Instanced)
	TArray<TObjectPtr<UBEItemDataFragment>> Fragments;

public:
	const UBEItemDataFragment* FindFragmentByClass(TSubclassOf<UBEItemDataFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}
};
