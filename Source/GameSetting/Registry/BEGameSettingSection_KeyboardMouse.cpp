// Copyright Eigi Chin

#include "CommonInputSubsystem.h"
#include "BEGameSettingRegistry.h"
#include "Player/BELocalPlayer.h"

#include "GameSetting.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalar.h"
#include "GameSettingValueScalarDynamic.h"
#include "GameSettingCollection.h"
#include "GameSettingAction.h"
#include "GameSetting/BEGameDeviceSettings.h"
#include "GameSetting/BEGameSharedSettings.h"
#include "EditCondition/WhenCondition.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "BEGameplayTags.h"
#include "GameSetting/SettingValue/BESettingValueKeyConfig.h"
#include "PlayerMappableInputConfig.h"


#define LOCTEXT_NAMESPACE "BE"

////////////////////////////////////////
//	キーボードマウスセクション

void UBEGameSettingRegistry::AddKeyboardMouseSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* KeyboardMouse = NewObject<UGameSettingCollection>();
	KeyboardMouse->SetDevName(TEXT("KeyboardMouseSection"));
	KeyboardMouse->SetDisplayName(LOCTEXT("KeyboardMouseSection_Name", "Keyboard And Mouse"));
	Screen->AddSetting(KeyboardMouse);

	const TSharedRef<FWhenCondition> WhenPlatformSupportsMouseAndKeyboard = MakeShared<FWhenCondition>(
		[](const ULocalPlayer*, FGameSettingEditableState& InOutEditState)
	{
		const UCommonInputPlatformSettings* PlatformInput = UPlatformSettingsManager::Get().GetSettingsForPlatform<UCommonInputPlatformSettings>();
		if (!PlatformInput->SupportsInputType(ECommonInputType::MouseAndKeyboard))
		{
			InOutEditState.Kill(TEXT("Platform does not support mouse and keyboard"));
		}
	});

	//======================================
	//	マウスX軸感度設定
	//======================================
	{
		UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
		Setting->SetDevName(TEXT("MouseSensitivityYaw"));
		Setting->SetDisplayName(LOCTEXT("MouseSensitivityYaw_Name", "X-Axis Sensitivity"));
		Setting->SetDescriptionRichText(LOCTEXT("MouseSensitivityYaw_Description", "Sets the sensitivity of the mouse's horizontal (x) axis. With higher settings the camera will move faster when looking left and right with the mouse."));

		Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityX));
		Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityX));
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetMouseSensitivityX());
		Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
		Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.01);
		Setting->SetMinimumLimit(0.01);

		Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

		KeyboardMouse->AddSetting(Setting);
	}

	//======================================
	//	マウスY軸感度設定
	//======================================
	{
		UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
		Setting->SetDevName(TEXT("MouseSensitivityPitch"));
		Setting->SetDisplayName(LOCTEXT("MouseSensitivityPitch_Name", "Y-Axis Sensitivity"));
		Setting->SetDescriptionRichText(LOCTEXT("MouseSensitivityPitch_Description", "Sets the sensitivity of the mouse's vertical (y) axis. With higher settings the camera will move faster when looking up and down with the mouse."));

		Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityY));
		Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityY));
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetMouseSensitivityY());
		Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
		Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.01);
		Setting->SetMinimumLimit(0.01);

		Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

		KeyboardMouse->AddSetting(Setting);
	}

	//======================================
	//	照準時の感度倍率設定
	//======================================
	{
		UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
		Setting->SetDevName(TEXT("MouseTargetingMultiplier"));
		Setting->SetDisplayName(LOCTEXT("MouseTargetingMultiplier_Name", "Targeting Sensitivity"));
		Setting->SetDescriptionRichText(LOCTEXT("MouseTargetingMultiplier_Description", "Sets the modifier for reducing mouse sensitivity when targeting. 100% will have no slow down when targeting. Lower settings will have more slow down when targeting."));

		Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetTargetingMultiplier));
		Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetTargetingMultiplier));
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetTargetingMultiplier());
		Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
		Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.01);
		Setting->SetMinimumLimit(0.01);

		Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

		KeyboardMouse->AddSetting(Setting);
	}
}

#undef LOCTEXT_NAMESPACE
