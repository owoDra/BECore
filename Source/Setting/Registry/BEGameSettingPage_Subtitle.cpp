// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameSettingRegistry.h"

#include "Player/BELocalPlayer.h"
#include "Setting/BESettingsShared.h"

#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "GameSettingCollection.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"

#define LOCTEXT_NAMESPACE "BE"

////////////////////////////////////////
//	字幕設定ページ

void UBEGameSettingRegistry::AddSubtitleSettingsPage(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollectionPage* SubtitlePage = NewObject<UGameSettingCollectionPage>();
	SubtitlePage->SetDevName(TEXT("SubtitlePage"));
	SubtitlePage->SetDisplayName(LOCTEXT("SubtitlePage_Name", "Subtitles"));
	SubtitlePage->SetDescriptionRichText(LOCTEXT("SubtitlePage_Description", "Configure the visual appearance of subtitles."));
	SubtitlePage->SetNavigationText(LOCTEXT("SubtitlePage_Navigation", "Edit"));

	SubtitlePage->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

	Screen->AddSetting(SubtitlePage);

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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetSubtitlesEnabled());

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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetSubtitlesTextSize());
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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetSubtitlesTextColor());
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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetSubtitlesTextBorder());
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
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetSubtitlesBackgroundOpacity());
			Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::Clear, LOCTEXT("ESubtitleBackgroundOpacity_Clear", "Clear"));
			Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::Low, LOCTEXT("ESubtitleBackgroundOpacity_Low", "Low"));
			Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::Medium, LOCTEXT("ESubtitleBackgroundOpacity_Medium", "Medium"));
			Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::High, LOCTEXT("ESubtitleBackgroundOpacity_High", "High"));
			Setting->AddEnumOption(ESubtitleDisplayBackgroundOpacity::Solid, LOCTEXT("ESubtitleBackgroundOpacity_Solid", "Solid"));

			SubtitleCollection->AddSetting(Setting);
		}
	}
}

#undef LOCTEXT_NAMESPACE
