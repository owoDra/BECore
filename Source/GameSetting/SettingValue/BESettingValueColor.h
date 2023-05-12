// Copyright Eigi Chin

#pragma once

#include "GameSettingValue.h"

#include "UObject/UObjectGlobals.h"
#include "Containers/UnrealString.h"
#include "Internationalization/Text.h"
#include "Math/Color.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Optional.h"

#include "BESettingValueColor.generated.h"

class UObject;
struct FFrame;

//////////////////////////////////////////////////////////////////////

UCLASS()
class UBESettingValueColor : public UGameSettingValue
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBESettingValueColor();

	//======================================
	//	操作
	//======================================
public:
	virtual TOptional<FLinearColor> GetDefaultValue() const															PURE_VIRTUAL(, return TOptional<FLinearColor>(););
	virtual void SetValue(FLinearColor Value, EGameSettingChangeReason Reason = EGameSettingChangeReason::Change)	PURE_VIRTUAL(, );
	virtual FLinearColor GetValue() const																			PURE_VIRTUAL(, return FLinearColor(););

	//======================================
	//	ユーティリティ
	//======================================
public:
	void SetValueFromHEXString(const FString& InString);

	FString GetHEXStringValue() const;

	virtual FText GetFormattedText() const;

	virtual FString GetAnalyticsValue() const override
	{
		return GetValue().ToString();
	}
};
