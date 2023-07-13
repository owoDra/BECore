// Copyright owoDra

#include "BEGameSettingRegistry.h"

#include "Player/BELocalPlayer.h"
#include "GameSetting/BEGameSharedSettings.h"
#include "GameSetting/SettingValue/BESettingValueDiscrete_Language.h"

#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "GameSettingCollection.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "EditCondition/WhenCondition.h"


#define LOCTEXT_NAMESPACE "BE"

void UBEGameSettingRegistry::AddAccessibilitySettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer)
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
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetColorBlindMode());
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
		Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetColorBlindStrength());
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
	{
		UGameSettingCollectionPage* SubtitlePage = NewObject<UGameSettingCollectionPage>();
		SubtitlePage->SetDevName(TEXT("SubtitlePage"));
		SubtitlePage->SetDisplayName(LOCTEXT("SubtitlePage_Name", "Subtitles"));
		SubtitlePage->SetDescriptionRichText(LOCTEXT("SubtitlePage_Description", "Configure the visual appearance of subtitles."));
		SubtitlePage->SetNavigationText(LOCTEXT("SubtitlePage_Navigation", "Edit"));

		SubtitlePage->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

		Accessibility->AddSetting(SubtitlePage);

		////////////////////////////////////////
		//	字幕設定
		{
			UGameSettingCollection* SubtitleCollection = NewObject<UGameSettingCollection>();
			SubtitleCollection->SetDevName(TEXT("SubtitlesSection"));
			SubtitleCollection->SetDisplayName(LOCTEXT("SubtitlesSection_Name", "Subtitles"));
			SubtitlePage->AddSetting(SubtitleCollection);

			//======================================
			//	字幕表示設定
			//======================================
			{
				UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
				Setting->SetDevName(TEXT("Subtitles"));
				Setting->SetDisplayName(LOCTEXT("Subtitles_Name", "Subtitles"));
				Setting->SetDescriptionRichText(LOCTEXT("Subtitles_Description", "Turns subtitles on/off."));

				Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesEnabled));
				Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesEnabled));
				Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetSubtitlesEnabled());

				SubtitleCollection->AddSetting(Setting);
			}

			//======================================
			//	字幕のサイズ設定
			//======================================
			{
				UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
				Setting->SetDevName(TEXT("SubtitleTextSize"));
				Setting->SetDisplayName(LOCTEXT("SubtitleTextSize_Name", "Text Size"));
				Setting->SetDescriptionRichText(LOCTEXT("SubtitleTextSize_Description", "Choose different sizes of the the subtitle text."));

				Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextSize));
				Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextSize));
				Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetSubtitlesTextSize());
				Setting->AddEnumOption(ESubtitleDisplayTextSize::ExtraSmall, LOCTEXT("ESubtitleTextSize_ExtraSmall", "Extra Small"));
				Setting->AddEnumOption(ESubtitleDisplayTextSize::Small, LOCTEXT("ESubtitleTextSize_Small", "Small"));
				Setting->AddEnumOption(ESubtitleDisplayTextSize::Medium, LOCTEXT("ESubtitleTextSize_Medium", "Medium"));
				Setting->AddEnumOption(ESubtitleDisplayTextSize::Large, LOCTEXT("ESubtitleTextSize_Large", "Large"));
				Setting->AddEnumOption(ESubtitleDisplayTextSize::ExtraLarge, LOCTEXT("ESubtitleTextSize_ExtraLarge", "Extra Large"));

				SubtitleCollection->AddSetting(Setting);
			}

			//======================================
			//	字幕の色設定
			//======================================
			{
				UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
				Setting->SetDevName(TEXT("SubtitleTextColor"));
				Setting->SetDisplayName(LOCTEXT("SubtitleTextColor_Name", "Text Color"));
				Setting->SetDescriptionRichText(LOCTEXT("SubtitleTextColor_Description", "Choose different colors for the subtitle text."));

				Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextColor));
				Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextColor));
				Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetSubtitlesTextColor());
				Setting->AddEnumOption(ESubtitleDisplayTextColor::White, LOCTEXT("ESubtitleTextColor_White", "White"));
				Setting->AddEnumOption(ESubtitleDisplayTextColor::Yellow, LOCTEXT("ESubtitleTextColor_Yellow", "Yellow"));

				SubtitleCollection->AddSetting(Setting);
			}

			//======================================
			//	字幕の縁取り設定
			//======================================
			{
				UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
				Setting->SetDevName(TEXT("SubtitleTextBorder"));
				Setting->SetDisplayName(LOCTEXT("SubtitleBackgroundStyle_Name", "Text Border"));
				Setting->SetDescriptionRichText(LOCTEXT("SubtitleTextBorder_Description", "Choose different borders for the text."));

				Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextBorder));
				Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextBorder));
				Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetSubtitlesTextBorder());
				Setting->AddEnumOption(ESubtitleDisplayTextBorder::None, LOCTEXT("ESubtitleTextBorder_None", "None"));
				Setting->AddEnumOption(ESubtitleDisplayTextBorder::Outline, LOCTEXT("ESubtitleTextBorder_Outline", "Outline"));
				Setting->AddEnumOption(ESubtitleDisplayTextBorder::DropShadow, LOCTEXT("ESubtitleTextBorder_DropShadow", "Drop Shadow"));

				SubtitleCollection->AddSetting(Setting);
			}

			//======================================
			//	字幕の透明度設定
			//======================================
			{
				UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
				Setting->SetDevName(TEXT("SubtitleBackgroundOpacity"));
				Setting->SetDisplayName(LOCTEXT("SubtitleBackground_Name", "Background Opacity"));
				Setting->SetDescriptionRichText(LOCTEXT("SubtitleBackgroundOpacity_Description", "Choose a different background or letterboxing for the subtitles."));

				Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesBackgroundOpacity));
				Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesBackgroundOpacity));
				Setting->SetDefaultValue(GetDefault<UBEGameSharedSettings>()->GetSubtitlesBackgroundOpacity());
				Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::Clear, LOCTEXT("ESubtitleBackgroundOpacity_Clear", "Clear"));
				Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::Low, LOCTEXT("ESubtitleBackgroundOpacity_Low", "Low"));
				Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::Medium, LOCTEXT("ESubtitleBackgroundOpacity_Medium", "Medium"));
				Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::High, LOCTEXT("ESubtitleBackgroundOpacity_High", "High"));
				Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::Solid, LOCTEXT("ESubtitleBackgroundOpacity_Solid", "Solid"));

				SubtitleCollection->AddSetting(Setting);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
