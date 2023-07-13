// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BESettingValueDiscrete_OverallQuality.h"

#include "GameSetting/BEGameDeviceSettings.h"

#include "CoreTypes.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "GameSettingFilterState.h"
#include "Internationalization/Internationalization.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingValueDiscrete_OverallQuality)

/////////////////////////////////////////////////////s

#define LOCTEXT_NAMESPACE "BESettings"

UBESettingValueDiscrete_OverallQuality::UBESettingValueDiscrete_OverallQuality()
{
}

void UBESettingValueDiscrete_OverallQuality::OnInitialized()
{
	Super::OnInitialized();

	UBEGameDeviceSettings* UserSettings = UBEGameDeviceSettings::Get();
	const int32 MaxQualityLevel = UserSettings->GetMaxSupportedOverallQualityLevel();

	auto AddOptionIfPossible = [&](int Index, FText&& Value) { if ((MaxQualityLevel < 0) || (Index <= MaxQualityLevel)) { Options.Add(Value); }};

	AddOptionIfPossible(0, LOCTEXT("VideoQualityOverall_Low", "Low"));
	AddOptionIfPossible(1, LOCTEXT("VideoQualityOverall_Medium", "Medium"));
	AddOptionIfPossible(2, LOCTEXT("VideoQualityOverall_High", "High"));
	AddOptionIfPossible(3, LOCTEXT("VideoQualityOverall_Epic", "Hyper"));

	OptionsWithCustom = Options;
	OptionsWithCustom.Add(LOCTEXT("VideoQualityOverall_Custom", "Custom"));
}

void UBESettingValueDiscrete_OverallQuality::StoreInitial()
{
}

void UBESettingValueDiscrete_OverallQuality::ResetToDefault()
{
}

void UBESettingValueDiscrete_OverallQuality::RestoreToInitial()
{
}

void UBESettingValueDiscrete_OverallQuality::SetDiscreteOptionByIndex(int32 Index)
{
	UGameUserSettings* UserSettings = CastChecked<UGameUserSettings>(GEngine->GetGameUserSettings());

	if (Index == GetCustomOptionIndex())
	{
		// Leave everything as is we're in a custom setup.
	}
	else
	{
		// Low / Medium / High / Hyper
		UserSettings->SetOverallScalabilityLevel(Index);
	}

	NotifySettingChanged(EGameSettingChangeReason::Change);
}

int32 UBESettingValueDiscrete_OverallQuality::GetDiscreteOptionIndex() const
{
	const int32 OverallQualityLevel = GetOverallQualityLevel();
	if (OverallQualityLevel == INDEX_NONE)
	{
		return GetCustomOptionIndex();
	}

	return OverallQualityLevel;
}

TArray<FText> UBESettingValueDiscrete_OverallQuality::GetDiscreteOptions() const
{
	const int32 OverallQualityLevel = GetOverallQualityLevel();
	if (OverallQualityLevel == INDEX_NONE)
	{
		return OptionsWithCustom;
	}
	else
	{
		return Options;
	}
}

int32 UBESettingValueDiscrete_OverallQuality::GetCustomOptionIndex() const
{
	return OptionsWithCustom.Num() - 1;
}

int32 UBESettingValueDiscrete_OverallQuality::GetOverallQualityLevel() const
{
	const UGameUserSettings* UserSettings = CastChecked<const UGameUserSettings>(GEngine->GetGameUserSettings());
	return UserSettings->GetOverallScalabilityLevel();
}

#undef LOCTEXT_NAMESPACE