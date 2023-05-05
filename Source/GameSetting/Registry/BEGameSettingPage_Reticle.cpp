// Copyright Eigi Chin

#include "BEGameSettingRegistry.h"

#include "Player/BELocalPlayer.h"
#include "GameSetting/BESettingsShared.h"
#include "GameSetting/CustomSetting/BESettingValueColorDynamic.h"

#include "GameSettingValueScalarDynamic.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingCollection.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"

#define LOCTEXT_NAMESPACE "BE"

////////////////////////////////////////
//	レティクル設定ページ

void UBEGameSettingRegistry::AddReticleSettingsPage(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollectionPage* ReticlePage = NewObject<UGameSettingCollectionPage>();
	ReticlePage->SetDevName(TEXT("ReticlePage"));
	ReticlePage->SetDisplayName(LOCTEXT("ReticlePage_Name", "Reticle"));
	ReticlePage->SetDescriptionRichText(LOCTEXT("ReticlePage_Description", "Configure the visual appearance of reticle."));
	ReticlePage->SetNavigationText(LOCTEXT("ReticlePage_Navigation", "Edit"));

	ReticlePage->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

	Screen->AddSetting(ReticlePage);

	UGameSetting* ReticlePreset = nullptr;

	////////////////////////////////////////
	//	レティクル設定
	{
		UGameSettingCollection* ReticleCollection = NewObject<UGameSettingCollection>();
		ReticleCollection->SetDevName(TEXT("ReticleSection"));
		ReticleCollection->SetDisplayName(LOCTEXT("ReticleSection_Name", "Reticle"));
		ReticlePage->AddSetting(ReticleCollection);

		//======================================
		//	レティクルスタイル設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
			Setting->SetDevName(TEXT("ReticleStyle"));
			Setting->SetDisplayName(LOCTEXT("ReticleStyle_Name", "Reticle Style"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleStyle_Description", 
				"Choose which reticle to display depending on the weapon.\n\
			<strong>Default</>: Use reticles per weapon instead of custom reticles\n\
			<strong>Auto</>: Use custom reticles automatically depending on the situation\n\
			<strong>Custom</>: Use custom reticles for all"
			));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetReticleStyle));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetReticleStyle));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetReticleStyle());
			Setting->AddEnumOption(EReticleStyle::Default, LOCTEXT("EReticleStyle_Default", "Default"));
			Setting->AddEnumOption(EReticleStyle::Auto, LOCTEXT("EReticleStyle_Auto", "Auto"));
			Setting->AddEnumOption(EReticleStyle::Custom, LOCTEXT("EReticleStyle_Custom", "Custom"));

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	ヒットマーカー表示設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("ReticleShowHitMarker"));
			Setting->SetDisplayName(LOCTEXT("ReticleShowHitMarker_Name", "Show Hit Marker"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleShowHitMarker_Description", "Choose to notify the UI when an attack hits the target."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetShowHitMarker));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetShowHitMarker));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetShowHitMarker());

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	エリミネーションマーカー表示設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("ReticleShowRliminationMarker"));
			Setting->SetDisplayName(LOCTEXT("ReticleShowRliminationMarker_Name", "Show Elimination Marker"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleShowRliminationMarker_Description", "Choose to notify the UI when an attack eliminate the target."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetShowEliminationMarker));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetShowEliminationMarker));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetShowEliminationMarker());

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	ファイアリングエラー設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("ReticleApplyFiringError"));
			Setting->SetDisplayName(LOCTEXT("ReticleApplyFiringError_Name", "Apply Firing Error"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleApplyFiringError_Description", "Choose whether to apply attack accuracy to the reticle"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetApplyFiringError));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetApplyFiringError));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetApplyFiringError());

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	オーバーライドレティクルカラー設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("ReticleShouldOverrideColor"));
			Setting->SetDisplayName(LOCTEXT("ReticleShouldOverrideColor_Name", "Should Override Targeting Reticle Color"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleShouldOverrideColor_Description", "Choose whether to override reticle color on aim"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(ShouldOverrideTargetingReticleColor));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetOverrideTargetingReticleColor));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->ShouldOverrideTargetingReticleColor());

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	エイミングレティクルカラー設定
		//======================================
		{
			UBESettingValueColorDynamic* Setting = NewObject<UBESettingValueColorDynamic>();
			Setting->SetDevName(TEXT("ReticleTargetingColor"));
			Setting->SetDisplayName(LOCTEXT("ReticleTargetingColor_Name", "Ainimg Reticle Color"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleTargetingColor_Description", "Reticle color on Targeting to override"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetTargetingReticleColor));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetTargetingReticleColor));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetTargetingReticleColor());

			ReticleCollection->AddSetting(Setting);
		}
	}

	////////////////////////////////////////
	//	カスタムレティクル設定
	{
		UGameSettingCollection* ReticleCollection = NewObject<UGameSettingCollection>();
		ReticleCollection->SetDevName(TEXT("CustomReticleSection"));
		ReticleCollection->SetDisplayName(LOCTEXT("CustomReticleSection_Name", "CustomReticle"));
		ReticlePage->AddSetting(ReticleCollection);

		//======================================
		//	レティクル番号設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("ReticleIndex"));
			Setting->SetDisplayName(LOCTEXT("ReticleIndex_Name", "Custom Reticle Presets"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleIndex_Description", "Choose which reticle preset to use and edit."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetReticleIndex));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetReticleIndex));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetReticleIndex());
			Setting->AddOption(0, LOCTEXT("ReticlePresetOne", "Preset 1"));
			Setting->AddOption(1, LOCTEXT("ReticlePresetTwo", "Preset 2"));
			Setting->AddOption(2, LOCTEXT("ReticlePresetThree", "Preset 3"));
			Setting->AddOption(3, LOCTEXT("ReticlePresetFour", "Preset 4"));
			Setting->AddOption(4, LOCTEXT("ReticlePresetFive", "Preset 5"));

			ReticleCollection->AddSetting(Setting);

			ReticlePreset = Setting;
		}

		//======================================
		//	ドットカラー設定
		//======================================
		{
			UBESettingValueColorDynamic* Setting = NewObject<UBESettingValueColorDynamic>();
			Setting->SetDevName(TEXT("ReticleDotColor"));
			Setting->SetDisplayName(LOCTEXT("ReticleDotColor_Name", "Dot Color"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleDotColor_Description", "Dot color of reticle"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetDotColor));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetDotColor));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetDotColor());

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	インラインカラー設定
		//======================================
		{
			UBESettingValueColorDynamic* Setting = NewObject<UBESettingValueColorDynamic>();
			Setting->SetDevName(TEXT("ReticleInlineColor"));
			Setting->SetDisplayName(LOCTEXT("ReticleInlineColor_Name", "Inline Color"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleInlineColor_Description", "Inline color of reticle"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInlineColor));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInlineColor));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetInlineColor());

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	アウトラインカラー設定
		//======================================
		{
			UBESettingValueColorDynamic* Setting = NewObject<UBESettingValueColorDynamic>();
			Setting->SetDevName(TEXT("ReticleOutlineColor"));
			Setting->SetDisplayName(LOCTEXT("ReticleOutlineColor_Name", "Outline Color"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleOutlineColor_Description", "Outline color of reticle"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetOutlineColor));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetOutlineColor));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetOutlineColor());

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	センタードット表示設定
		//======================================
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("ReticleShowCenterDot"));
			Setting->SetDisplayName(LOCTEXT("ReticleShowCenterDot_Name", "Show Center Dot"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleShowCenterDot_Description", "Choose whether to display the center dot"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetShowCenterDot));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetShowCenterDot));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetShowCenterDot());

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	センタードットサイズ設定
		//======================================
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ReticleCenterDotSize"));
			Setting->SetDisplayName(LOCTEXT("ReticleCenterDotSize_Name", "Center Dot Size"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleCenterDotSize_Description", "Size of center dot"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetDotSize));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetDotSize));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetDotSize());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::SourceAsInteger);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 16), 1);

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	アウトラインの厚み設定
		//======================================
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ReticleOutlineThickness"));
			Setting->SetDisplayName(LOCTEXT("ReticleOutlineThickness_Name", "Outline Thickness"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleOutlineThickness_Description", "Thickness of outline"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetOutlineThickness));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetOutlineThickness));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetOutlineThickness());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::SourceAsInteger);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 16), 1);

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	横インラインの厚み設定
		//======================================
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ReticleInlineThicknessHori"));
			Setting->SetDisplayName(LOCTEXT("ReticleInlineThicknessHori_Name", "Horizontal Inline Thickness"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleInlineThicknessHori_Description", "Thickness of horizontal inline"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetLineThicknessHori));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetLineThicknessHori));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetLineThicknessHori());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::SourceAsInteger);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 16), 1);

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	横インラインの長さ設定
		//======================================
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ReticleInlineLengthHori"));
			Setting->SetDisplayName(LOCTEXT("ReticleInlineLengthHori_Name", "Horizontal Inline Length"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleInlineLengthHori_Description", "Length of horizontal inline"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetLineLengthHori));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetLineLengthHori));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetLineLengthHori());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::SourceAsInteger);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 32), 1);

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	横インラインのオフセット設定
		//======================================
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ReticleInlineOffsetHori"));
			Setting->SetDisplayName(LOCTEXT("ReticleInlineOffsetHori_Name", "Horizontal Inline Offset"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleInlineOffsetHori_Description", "Offset of horizontal inline"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetLineOffsetHori));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetLineOffsetHori));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetLineOffsetHori());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::SourceAsInteger);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 64), 1);

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	縦インラインの厚み設定
		//======================================
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ReticleInlineThicknessVert"));
			Setting->SetDisplayName(LOCTEXT("ReticleInlineThicknessVert_Name", "Vertical Inline Thickness"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleInlineThicknessVert_Description", "Thickness of vertical inline"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetLineThicknessVert));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetLineThicknessVert));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetLineThicknessVert());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::SourceAsInteger);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 16), 1);

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	縦インラインの長さ設定
		//======================================
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ReticleInlineLengthVert"));
			Setting->SetDisplayName(LOCTEXT("ReticleInlineLengthVert_Name", "Vertical Inline Length"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleInlineLengthVert_Description", "Length of vertical inline"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetLineLengthVert));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetLineLengthVert));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetLineLengthVert());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::SourceAsInteger);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 32), 1);

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}

		//======================================
		//	縦インラインのオフセット設定
		//======================================
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ReticleInlineOffsetVert"));
			Setting->SetDisplayName(LOCTEXT("ReticleInlineOffsetVert_Name", "Vertical Inline Offset"));
			Setting->SetDescriptionRichText(LOCTEXT("ReticleInlineOffsetVert_Description", "Offset of vertical inline"));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetLineOffsetVert));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetLineOffsetVert));
			Setting->SetDefaultValue(GetDefault<UBESettingsShared>()->GetLineOffsetVert());
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::SourceAsInteger);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 64), 1);

			Setting->AddEditDependency(ReticlePreset);

			ReticleCollection->AddSetting(Setting);
		}
	}
}

#undef LOCTEXT_NAMESPACE