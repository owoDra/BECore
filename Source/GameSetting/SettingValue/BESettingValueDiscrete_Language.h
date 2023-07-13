// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameSettingValueDiscrete.h"

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "HAL/Platform.h"
#include "UObject/UObjectGlobals.h"

#include "BESettingValueDiscrete_Language.generated.h"

class FText;
class UObject;

/////////////////////////////////////////////////////

UCLASS()
class BECORE_API UBESettingValueDiscrete_Language : public UGameSettingValueDiscrete
{
	GENERATED_BODY()
	
public:

	UBESettingValueDiscrete_Language();

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
	virtual void OnApply() override;

protected:
	TArray<FString> AvailableCultureNames;
};