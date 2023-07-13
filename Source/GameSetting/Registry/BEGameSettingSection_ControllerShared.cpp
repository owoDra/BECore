// Copyright owoDra

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
//	共通入力セクション

void UBEGameSettingRegistry::AddControllerSharedSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* ControllerShared = NewObject<UGameSettingCollection>();
	ControllerShared->SetDevName(TEXT("ControllerSharedSection"));
	ControllerShared->SetDisplayName(LOCTEXT("ControllerSharedSection_Name", "Shared"));
	Screen->AddSetting(ControllerShared);

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
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetInvertVerticalAxis());

		ControllerShared->AddSetting(Setting);
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
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetInvertHorizontalAxis());

		ControllerShared->AddSetting(Setting);
	}
}

#undef LOCTEXT_NAMESPACE
