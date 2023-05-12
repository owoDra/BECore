// Copyright Eigi Chin

#pragma once

#include "BESettingValueColor.h"

#include "GameSettingFilterState.h"
#include "Internationalization/Text.h"
#include "Math/Color.h"
#include "Math/Range.h"
#include "Misc/Optional.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"
#include "UObject/UObjectGlobals.h"

#include "BESettingValueColorDynamic.generated.h"

class FGameSettingDataSource;
class UObject;

//////////////////////////////////////////////////////////////////////

UCLASS()
class UBESettingValueColorDynamic : public UBESettingValueColor
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBESettingValueColorDynamic();

	virtual void Startup() override;
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;

	void SetDynamicGetter(const TSharedRef<FGameSettingDataSource>& InGetter);
	void SetDynamicSetter(const TSharedRef<FGameSettingDataSource>& InSetter);
	void SetDefaultValue(FLinearColor InValue);

	virtual void OnInitialized() override;

	void OnDataSourcesReady();

protected:

	TSharedPtr<FGameSettingDataSource> Getter;
	TSharedPtr<FGameSettingDataSource> Setter;

	TOptional<FLinearColor> DefaultValue;
	FLinearColor InitialValue = FLinearColor();

	//======================================
	//	操作
	//======================================
public:
	virtual TOptional<FLinearColor> GetDefaultValue() const override;
	virtual void SetValue(FLinearColor Value, EGameSettingChangeReason Reason = EGameSettingChangeReason::Change) override;
	virtual FLinearColor GetValue() const override;
};
