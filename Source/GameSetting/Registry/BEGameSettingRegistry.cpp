// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameSettingRegistry.h"

#include "Player/BELocalPlayer.h"
#include "GameSetting/BESettingsLocal.h"
#include "GameSetting/BESettingsShared.h"

#include "Engine/LocalPlayer.h"
#include "GameSettingCollection.h"
#include "HAL/Platform.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameSettingRegistry)

DEFINE_LOG_CATEGORY(LogBEGameSettingRegistry);

///////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BE"

UBEGameSettingRegistry::UBEGameSettingRegistry()
{
}

UBEGameSettingRegistry* UBEGameSettingRegistry::Get(UBELocalPlayer* InLocalPlayer)
{
	UBEGameSettingRegistry* Registry = FindObject<UBEGameSettingRegistry>(InLocalPlayer, TEXT("BEGameSettingRegistry"), true);
	if (Registry == nullptr)
	{
		Registry = NewObject<UBEGameSettingRegistry>(InLocalPlayer, TEXT("BEGameSettingRegistry"));
		Registry->Initialize(InLocalPlayer);
	}

	return Registry;
}

bool UBEGameSettingRegistry::IsFinishedInitializing() const
{
	if (Super::IsFinishedInitializing())
	{
		if (UBELocalPlayer* LocalPlayer = Cast<UBELocalPlayer>(OwningLocalPlayer))
		{
			if (LocalPlayer->GetSharedSettings() == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

void UBEGameSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	if (UBELocalPlayer* BELocalPlayer = Cast<UBELocalPlayer>(InLocalPlayer))
	{
		/**
		 * コントロール設定カテゴリー
		 */
		ControlSettings = InitializeControlSettings(BELocalPlayer);
		RegisterSetting(ControlSettings);


		/**
		 * ビデオ設定カテゴリー
		 */
		VideoSettings = InitializeVideoSettings(BELocalPlayer);
		RegisterSetting(VideoSettings);


		/**
		 * ゲームプレイ設定カテゴリー
		 */
		GameplaySettings = InitializeGameplaySettings(BELocalPlayer);
		RegisterSetting(GameplaySettings);


		/**
		 * オーディオ設定カテゴリー
		 */
		AudioSettings = InitializeAudioSettings(BELocalPlayer);
		RegisterSetting(AudioSettings);
	}
}

void UBEGameSettingRegistry::SaveChanges()
{
	Super::SaveChanges();
	
	if (UBELocalPlayer* LocalPlayer = Cast<UBELocalPlayer>(OwningLocalPlayer))
	{
		// Game user settings need to be applied to handle things like resolution, this saves indirectly
		LocalPlayer->GetLocalSettings()->ApplySettings(false);
		
		LocalPlayer->GetSharedSettings()->ApplySettings();
		LocalPlayer->GetSharedSettings()->SaveSettings();
	}
}

#undef LOCTEXT_NAMESPACE
