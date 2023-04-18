// Copyright Eigi Chin

#include "BESettingValueColor.h"

#include "Math/UnrealMathUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingValueColor)

//////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BESettings"

UBESettingValueColor::UBESettingValueColor()
{
}

void UBESettingValueColor::SetValueFromHEXString(const FString& InString)
{
	SetValue(FLinearColor(FColor::FromHex(InString)));
}

FString UBESettingValueColor::GetHEXStringValue() const
{
	return GetValue().ToFColor(true).ToHex();
}

FText UBESettingValueColor::GetFormattedText() const
{
	return FText::FromString(GetHEXStringValue());
}

#undef LOCTEXT_NAMESPACE
