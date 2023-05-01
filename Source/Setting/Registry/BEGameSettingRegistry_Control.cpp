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
#include "Setting/BESettingsLocal.h"
#include "Setting/BESettingsShared.h"
#include "EditCondition/WhenCondition.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "BEGameplayTags.h"
#include "Setting/CustomSetting/BESettingValueKeyConfig.h"
#include "PlayerMappableInputConfig.h"

#define LOCTEXT_NAMESPACE "BE"


////////////////////////////////////////////////////////////
//	コントロール設定

UGameSettingCollection* UBEGameSettingRegistry::InitializeControlSettings(UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("ControlCollection"));
	Screen->SetDisplayName(LOCTEXT("ControlCollection_Name", "Control"));
	Screen->Initialize(InLocalPlayer);

	///////////////////////////////////////////////////////
	//	キーバインド設定

	AddKeyBindSettingsPage(Screen, InLocalPlayer);


	///////////////////////////////////////////////////////
	//	キーボードマウス設定
	{
		UGameSettingCollection* KeyboardMouse = NewObject<UGameSettingCollection>();
		KeyboardMouse->SetDevName(TEXT("KeyboardMouseSection"));
		KeyboardMouse->SetDisplayName(LOCTEXT("KeyboardMouseSection_Name", "Keyboard & Mouse"));
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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetMouseSensitivityX());
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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetMouseSensitivityY());
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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetTargetingMultiplier());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::RawTwoDecimals);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.01);
			Setting->SetMinimumLimit(0.01);

			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);

			KeyboardMouse->AddSetting(Setting);
		}
	}

	///////////////////////////////////////////////////////
	//	コントローラー設定
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
			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetControllerPlatform));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetControllerPlatform));
			
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

					const FName CurrentControllerPlatform = GetDefault<UBESettingsLocal>()->GetControllerPlatform();
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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetForceFeedbackEnabled());

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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetGamepadLookSensitivityPreset());

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
			Setting->SetDescriptionRichText(LOCTEXT("LookSensitivityPresetAds_Description", "How quickly your view rotates while Targeting down sights (ADS)."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadTargetingSensitivityPreset));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadTargetingSensitivityPreset));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetGamepadTargetingSensitivityPreset());

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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetGamepadMoveStickDeadZone());
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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetGamepadLookStickDeadZone());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);
			Setting->SetMinimumLimit(0.05);
			Setting->SetMaximumLimit(0.95);

			Gamepad->AddSetting(Setting);
		}
	}

	///////////////////////////////////////////////////////
	//	共通設定
	{
		UGameSettingCollection* Shared = NewObject<UGameSettingCollection>();
		Shared->SetDevName(TEXT("SharedSection"));
		Shared->SetDisplayName(LOCTEXT("SharedSection_Name", "Shared"));
		Screen->AddSetting(Shared);

		//======================================
		//	Y軸を反転
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("InvertVerticalAxis"));
			Setting->SetDisplayName(LOCTEXT("InvertVerticalAxis_Name", "Invert Vertical Axis"));
			Setting->SetDescriptionRichText(LOCTEXT("InvertVerticalAxis_Description", "Enable the inversion of the vertical look axis."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertVerticalAxis));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertVerticalAxis));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetInvertVerticalAxis());

			Shared->AddSetting(Setting);
		}

		//======================================
		//	X軸を反転
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("InvertHorizontalAxis"));
			Setting->SetDisplayName(LOCTEXT("InvertHorizontalAxis_Name", "Invert Horizontal Axis"));
			Setting->SetDescriptionRichText(LOCTEXT("InvertHorizontalAxis_Description", "Enable the inversion of the Horizontal look axis."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertHorizontalAxis));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertHorizontalAxis));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetInvertHorizontalAxis());

			Shared->AddSetting(Setting);
		}

		//======================================
		//	デフォルト移動モード設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("DefaultRun"));
			Setting->SetDisplayName(LOCTEXT("DefaultRun_Name", "Default Movement Mode"));
			Setting->SetDescriptionRichText(LOCTEXT("DefaultRun_Description", "Choose movement mode."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetDefaultRun));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetDefaultRun));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetDefaultRun());

			Setting->SetTrueText(LOCTEXT("DefaultRun_True", "Run"));
			Setting->SetFalseText(LOCTEXT("DefaultRun_False", "Walk"));

			Shared->AddSetting(Setting);
		}

		//======================================
		//	トグルADS設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("UseToggleADS"));
			Setting->SetDisplayName(LOCTEXT("UseToggleADS_Name", "Use Toggle ADS"));
			Setting->SetDescriptionRichText(LOCTEXT("UseToggleADS_Description", "Use ADS with toggle input."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetUseToggleADS));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetUseToggleADS));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetUseToggleADS());

			Shared->AddSetting(Setting);
		}

		//======================================
		//	トグル走り設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("UseToggleRun"));
			Setting->SetDisplayName(LOCTEXT("UseToggleRun_Name", "Use Toggle Run"));
			Setting->SetDescriptionRichText(LOCTEXT("UseToggleRun_Description", "Use run with toggle input."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetUseToggleRun));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetUseToggleRun));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetUseToggleRun());

			Shared->AddSetting(Setting);
		}

		//======================================
		//	トグルしゃがみ設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("UseToggleCrouch"));
			Setting->SetDisplayName(LOCTEXT("UseToggleCrouch_Name", "Use Toggle Crouch"));
			Setting->SetDescriptionRichText(LOCTEXT("UseToggleCrouch_Description", "Use crouch with toggle input."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetUseToggleCrouch));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetUseToggleCrouch));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetUseToggleCrouch());

			Shared->AddSetting(Setting);
		}

		//======================================
		//	トグルHUDオーバーレイ設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("UseToggleOverlay"));
			Setting->SetDisplayName(LOCTEXT("UseToggleOverlay_Name", "Use Toggle HUD Overlay"));
			Setting->SetDescriptionRichText(LOCTEXT("UseToggleOverlay_Description", "Use HUD overlay with toggle input."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetUseToggleOverlay));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetUseToggleOverlay));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetUseToggleOverlay());

			Shared->AddSetting(Setting);
		}
	}

	return Screen;
}

#undef LOCTEXT_NAMESPACE
