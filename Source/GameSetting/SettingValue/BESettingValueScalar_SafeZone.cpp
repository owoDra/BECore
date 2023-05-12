// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BESettingValueScalar_SafeZone.h"

#include "GameSetting/Registry/BEGameSettingRegistry.h"
#include "Player/BELocalPlayer.h"

#include "DataSource/GameSettingDataSourceDynamic.h"
#include "HAL/Platform.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Misc/Optional.h"
#include "Widgets/Layout/SSafeZone.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingValueScalar_SafeZone)

class UGameSetting;

//////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BESettings"

UBESettingAction_SafeZoneEditor::UBESettingAction_SafeZoneEditor()
{
	SafeZoneValueSetting = NewObject<UBESettingValueScalarDynamic_SafeZoneValue>();
	SafeZoneValueSetting->SetDevName(TEXT("SafeZoneValue"));
	SafeZoneValueSetting->SetDisplayName(LOCTEXT("SafeZoneValue_Name", "Safe Zone Value"));
	SafeZoneValueSetting->SetDescriptionRichText(LOCTEXT("SafeZoneValue_Description", "The safezone area percentage."));
	SafeZoneValueSetting->SetDefaultValue(0.0f);
	SafeZoneValueSetting->SetDynamicGetter(GET_DEVICE_SETTINGS_FUNCTION_PATH(GetSafeZone));
	SafeZoneValueSetting->SetDynamicSetter(GET_DEVICE_SETTINGS_FUNCTION_PATH(SetSafeZone));
	SafeZoneValueSetting->SetDisplayFormat([](double SourceValue, double NormalizedValue){ return FText::AsNumber(SourceValue); });
	SafeZoneValueSetting->SetSettingParent(this);
}

TArray<UGameSetting*> UBESettingAction_SafeZoneEditor::GetChildSettings()
{
	return { SafeZoneValueSetting };
}

void UBESettingValueScalarDynamic_SafeZoneValue::ResetToDefault()
{
	Super::ResetToDefault();
	SSafeZone::SetGlobalSafeZoneScale(TOptional<float>(DefaultValue.Get(0.0f)));
}

void UBESettingValueScalarDynamic_SafeZoneValue::RestoreToInitial()
{
	Super::RestoreToInitial();
	SSafeZone::SetGlobalSafeZoneScale(TOptional<float>(InitialValue));
}

#undef LOCTEXT_NAMESPACE