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
#include "GameSetting/SettingValue/BESettingValueKeyConfig.h"
#include "PlayerMappableInputConfig.h"


#define LOCTEXT_NAMESPACE "BE"

////////////////////////////////////////
//	ゲームパッドセクション

void UBEGameSettingRegistry::AddGamepadSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Gamepad = NewObject<UGameSettingCollection>();
	Gamepad->SetDevName(TEXT("GamepadSection"));
	Gamepad->SetDisplayName(LOCTEXT("GamepadSection_Name", "Gamepad"));
	Screen->AddSetting(Gamepad);

	const FText EGamepadSensitivityText[] = {
			FText::GetEmpty(),
			LOCTEXT("EFortGamepadSensitivity_Slow", "1 (Slow)"),
			LOCTEXT("EFortGamepadSensitivity_SlowPlus", "2 (Slow+)"),
			LOCTEXT("EFortGamepadSensitivity_SlowPlusPlus", "3 (Slow++)"),
			LOCTEXT("EFortGamepadSensitivity_Normal", "4 (Normal)"),
			LOCTEXT("EFortGamepadSensitivity_NormalPlus", "5 (Normal+)"),
			LOCTEXT("EFortGamepadSensitivity_NormalPlusPlus", "6 (Normal++)"),
			LOCTEXT("EFortGamepadSensitivity_Fast", "7 (Fast)"),
			LOCTEXT("EFortGamepadSensitivity_FastPlus", "8 (Fast+)"),
			LOCTEXT("EFortGamepadSensitivity_FastPlusPlus", "9 (Fast++)"),
			LOCTEXT("EFortGamepadSensitivity_Insane", "10 (Insane)"),
	};

	//======================================
	//	コントローラーハードウェア設定
	//======================================
	{
		UGameSettingValueDiscreteDynamic* Setting = NewObject<UGameSettingValueDiscreteDynamic>();
		Setting->SetDevName(TEXT("ControllerHardware"));
		Setting->SetDisplayName(LOCTEXT("ControllerHardware_Name", "Controller Hardware"));
		Setting->SetDescriptionRichText(LOCTEXT("ControllerHardware_Description", "The type of controller you're using."));
		Setting->SetDynamicGetter(GET_DEVICE_SETTINGS_FUNCTION_PATH(GetControllerPlatform));
		Setting->SetDynamicSetter(GET_DEVICE_SETTINGS_FUNCTION_PATH(SetControllerPlatform));

		if (UCommonInputPlatformSettings* PlatformInputSettings = UPlatformSettingsManager::Get().GetSettingsForPlatform<UCommonInputPlatformSettings>())
		{
			const TArray<TSoftClassPtr<UCommonInputBaseControllerData>>& ControllerDatas = PlatformInputSettings->GetControllerData();
			for (TSoftClassPtr<UCommonInputBaseControllerData> ControllerDataPtr : ControllerDatas)
			{
				if (TSubclassOf<UCommonInputBaseControllerData> ControllerDataClass = ControllerDataPtr.LoadSynchronous())
				{
					const UCommonInputBaseControllerData* ControllerData = ControllerDataClass.GetDefaultObject();
					if (ControllerData->InputType == ECommonInputType::Gamepad)
					{
						Setting->AddDynamicOption(ControllerData->GamepadName.ToString(), ControllerData->GamepadDisplayName);
					}
				}
			}

			// Add the setting if we can select more than one game controller type on this platform
			// and we are allowed to change it
			if (Setting->GetDynamicOptions().Num() > 1 && PlatformInputSettings->CanChangeGamepadType())
			{
				Gamepad->AddSetting(Setting);

				const FName CurrentControllerPlatform = GetDefault<UBEGameDeviceSettings>()->GetControllerPlatform();
				if (CurrentControllerPlatform == NAME_None)
				{
					Setting->SetDiscreteOptionByIndex(0);
				}
				else
				{
					Setting->SetDefaultValueFromString(CurrentControllerPlatform.ToString());
				}
			}
		}
	}

	//======================================
	//	コントローラーの振動設定
	//======================================
	{
		UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
		Setting->SetDevName(TEXT("GamepadVibration"));
		Setting->SetDisplayName(LOCTEXT("GamepadVibration_Name", "Vibration"));
		Setting->SetDescriptionRichText(LOCTEXT("GamepadVibration_Description", "Turns controller vibration on/off."));

		Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetForceFeedbackEnabled));
		Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetForceFeedbackEnabled));
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetForceFeedbackEnabled());

		Gamepad->AddSetting(Setting);
	}

	//======================================
	//	コントローラー感度設定
	//======================================
	{
		UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
		Setting->SetDevName(TEXT("LookSensitivityPreset"));
		Setting->SetDisplayName(LOCTEXT("LookSensitivityPreset_Name", "Look Sensitivity"));
		Setting->SetDescriptionRichText(LOCTEXT("LookSensitivityPreset_Description", "How quickly your view rotates."));

		Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadLookSensitivityPreset));
		Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetLookSensitivityPreset));
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetGamepadLookSensitivityPreset());

		for (int32 PresetIndex = 1; PresetIndex < (int32)EBEGamepadSensitivity::MAX; PresetIndex++)
		{
			Setting->AddEnumOption(static_cast<EBEGamepadSensitivity>(PresetIndex), EGamepadSensitivityText[PresetIndex]);
		}

		Gamepad->AddSetting(Setting);
	}

	//======================================
	//	コントローラー照準感度設定
	//======================================
	{
		UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
		Setting->SetDevName(TEXT("LookSensitivityPresetAds"));
		Setting->SetDisplayName(LOCTEXT("LookSensitivityPresetAds_Name", "Aim Sensitivity (ADS)"));
		Setting->SetDescriptionRichText(LOCTEXT("LookSensitivityPresetAds_Description", "How quickly your view rotates while aiming down sights (ADS)."));

		Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadTargetingSensitivityPreset));
		Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadTargetingSensitivityPreset));
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetGamepadTargetingSensitivityPreset());

		for (int32 PresetIndex = 1; PresetIndex < (int32)EBEGamepadSensitivity::MAX; PresetIndex++)
		{
			Setting->AddEnumOption(static_cast<EBEGamepadSensitivity>(PresetIndex), EGamepadSensitivityText[PresetIndex]);
		}

		Gamepad->AddSetting(Setting);
	}

	//======================================
	//	左スティックデッドゾーン設定
	//======================================
	{
		UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
		Setting->SetDevName(TEXT("MoveStickDeadZone"));
		Setting->SetDisplayName(LOCTEXT("MoveStickDeadZone_Name", "Left Stick DeadZone"));
		Setting->SetDescriptionRichText(LOCTEXT("MoveStickDeadZone_Description", "Increase or decrease the area surrounding the stick that we ignore input from.  Setting this value too low may result in the character continuing to move even after removing your finger from the stick."));

		Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadMoveStickDeadZone));
		Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadMoveStickDeadZone));
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetGamepadMoveStickDeadZone());
		Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);
		Setting->SetMinimumLimit(0.05);
		Setting->SetMaximumLimit(0.95);

		Gamepad->AddSetting(Setting);
	}

	//======================================
	//	右スティックデッドゾーン設定
	//======================================
	{
		UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
		Setting->SetDevName(TEXT("LookStickDeadZone"));
		Setting->SetDisplayName(LOCTEXT("LookStickDeadZone_Name", "Right Stick DeadZone"));
		Setting->SetDescriptionRichText(LOCTEXT("LookStickDeadZone_Description", "Increase or decrease the area surrounding the stick that we ignore input from.  Setting this value too low may result in the camera continuing to move even after removing your finger from the stick."));

		Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadLookStickDeadZone));
		Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadLookStickDeadZone));
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetGamepadLookStickDeadZone());
		Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);
		Setting->SetMinimumLimit(0.05);
		Setting->SetMaximumLimit(0.95);

		Gamepad->AddSetting(Setting);
	}
}

#undef LOCTEXT_NAMESPACE
