// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameSettingValueDiscrete.h"

#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "Internationalization/Text.h"
#include "UObject/UObjectGlobals.h"

#include "BESettingValueDiscrete_OverallQuality.generated.h"

class UObject;

/////////////////////////////////////////////////////

UCLASS()
class UBESettingValueDiscrete_OverallQuality : public UGameSettingValueDiscrete
{
	GENERATED_BODY()
	
public:

	UBESettingValueDiscrete_OverallQuality();

	/** UGameSettingValue */
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;

	/** UGameSettingValueDiscrete */
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;
	
	int32 GetCustomOptionIndex() const;
	int32 GetOverallQualityLevel() const;

	TArray<FText> Options;
	TArray<FText> OptionsWithCustom;

};