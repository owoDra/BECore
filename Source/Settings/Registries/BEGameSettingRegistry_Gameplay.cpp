// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameSettingRegistry.h"

#include "GameSetting.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalar.h"
#include "GameSettingValueScalarDynamic.h"
#include "GameSettingCollection.h"
#include "GameSettingAction.h"

#include "Settings/BESettingsLocal.h"
#include "Settings/BESettingsShared.h"

#include "Player/BELocalPlayer.h"

#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "EditCondition/WhenCondition.h"

#include "Settings/CustomSettings/BESettingValueDiscrete_PerfStat.h"
#include "Settings/CustomSettings/BESettingValueDiscrete_Language.h"

#define LOCTEXT_NAMESPACE "BE"

////////////////////////////////////////
//	ゲームプレイ設定

UGameSettingCollection* UBEGameSettingRegistry::InitializeGameplaySettings(UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("GameplayCollection"));
	Screen->SetDisplayName(LOCTEXT("GameplayCollection_Name", "Gameplay"));
	Screen->Initialize(InLocalPlayer);

	////////////////////////////////////////
	//	アクセシビリティ設定
	{
		UGameSettingCollection* Accessibility = NewObject<UGameSettingCollection>();
		Accessibility->SetDevName(TEXT("AccessibilitySection"));
		Accessibility->SetDisplayName(LOCTEXT("AccessibilitySection_Name", "Accessibility"));
		Screen->AddSetting(Accessibility);

		//======================================
		//	言語設定
		//======================================
		{
			UBESettingValueDiscrete_Language* Setting = NewObject<UBESettingValueDiscrete_Language>();
			Setting->SetDevName(TEXT("Language"));
			Setting->SetDisplayName(LOCTEXT("LanguageSetting_Name", "Language"));
			Setting->SetDescriptionRichText(LOCTEXT("LanguageSetting_Description", "The language of the game."));
			
#if WITH_EDITOR
			if (GIsEditor)
			{
				Setting->SetDescriptionRichText(LOCTEXT("LanguageSetting_WithEditor_Description", "The language of the game.\n\n<text color=\"#ffff00\">WARNING: Language changes will not affect PIE, you'll need to run with -game to test this, or change your PIE language options in the editor preferences.</>"));
			}
#endif
			
			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Accessibility->AddSetting(Setting);
		}

		//======================================
		//	色覚特性型設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
			Setting->SetDevName(TEXT("ColorBlindMode"));
			Setting->SetDisplayName(LOCTEXT("ColorBlindMode_Name", "Color Blind Mode"));
			Setting->SetDescriptionRichText(LOCTEXT("ColorBlindMode_Description", "Using the provided images, test out the different color blind modes to find a color correction that works best for you."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindMode));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindMode));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetColorBlindMode());
			Setting->AddEnumOption(EColorBlindMode::Off, LOCTEXT("ColorBlindRotatorSettingOff", "Off"));
			Setting->AddEnumOption(EColorBlindMode::Deuteranope, LOCTEXT("ColorBlindRotatorSettingDeuteranope", "Deuteranope"));
			Setting->AddEnumOption(EColorBlindMode::Protanope, LOCTEXT("ColorBlindRotatorSettingProtanope", "Protanope"));
			Setting->AddEnumOption(EColorBlindMode::Tritanope, LOCTEXT("ColorBlindRotatorSettingTritanope", "Tritanope"));

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Accessibility->AddSetting(Setting);
		}

		//======================================
		//	色覚特性強度設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("ColorBlindStrength"));
			Setting->SetDisplayName(LOCTEXT("ColorBlindStrength_Name", "Color Blind Strength"));
			Setting->SetDescriptionRichText(LOCTEXT("ColorBlindStrength_Description", "Using the provided images, test out the different strengths to find a color correction that works best for you."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindStrength));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindStrength));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetColorBlindStrength());
			for (int32 Index = 0; Index <= 10; Index++)
			{
				Setting->AddOption(Index, FText::AsNumber(Index));
			}

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Accessibility->AddSetting(Setting);
		}

		//======================================
		//	字幕設定
		//======================================

		AddSubtitleSettingsPage(Accessibility, InLocalPlayer);
	}

	////////////////////////////////////////
	//	HUD設定
	{
		UGameSettingCollection* HUD = NewObject<UGameSettingCollection>();
		HUD->SetDevName(TEXT("HUDSection"));
		HUD->SetDisplayName(LOCTEXT("HUDSection_Name", "HUD"));
		Screen->AddSetting(HUD);

		//======================================
		//	レティクル設定
		//======================================

		AddReticleSettingsPage(HUD, InLocalPlayer);
	}

	////////////////////////////////////////
	//	情報表示設定
	{
		UGameSettingCollection* Information = NewObject<UGameSettingCollection>();
		Information->SetDevName(TEXT("InformationSection"));
		Information->SetDisplayName(LOCTEXT("InformationSection_Name", "Information"));
		Screen->AddSetting(Information);

		//======================================
		//	パフォーマンス表示設定
		//======================================

		AddPrefStatSettingsPage(Information, InLocalPlayer);
	}

	return Screen;
}

#undef LOCTEXT_NAMESPACE
