// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameSettingValueDiscrete.h"

#include "Performance/BEPerformanceStatTypes.h"

#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "Internationalization/Text.h"
#include "UObject/UObjectGlobals.h"

#include "BESettingValueDiscrete_PerfStat.generated.h"

class UObject;

//////////////////////////////////////////////////////////////////////

UCLASS()
class UBESettingValueDiscrete_PerfStat : public UGameSettingValueDiscrete
{
	GENERATED_BODY()
	
public:

	UBESettingValueDiscrete_PerfStat();

	void SetStat(EBEDisplayablePerformanceStat InStat);

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
	
	void AddMode(FText&& Label, EBEStatDisplayMode Mode);
protected:
	TArray<FText> Options;
	TArray<EBEStatDisplayMode> DisplayModes;

	EBEDisplayablePerformanceStat StatToDisplay;
	EBEStatDisplayMode InitialMode;
};