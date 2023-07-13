// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEGameSettingRegistry.h"

#include "GameSetting/BEGameDeviceSettings.h"
#include "GameSetting/SettingValue/BESettingValueKeyConfig.h"
#include "Player/BELocalPlayer.h"

#include "GameSetting.h"
#include "GameSettingCollection.h"
#include "PlayerMappableInputConfig.h"


#define LOCTEXT_NAMESPACE "BE"

///////////////////////////////////////////////////////
//	キーバインド設定

void UBEGameSettingRegistry::AddKeyBindSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Keybind = NewObject<UGameSettingCollection>();
	Keybind->SetDevName(TEXT("KeybindSection"));
	Keybind->SetDisplayName(LOCTEXT("KeybindSection_Name", "Keybind"));
	Screen->AddSetting(Keybind);

	////////////////////////////////////////
	//	キーバインド設定ページ
	{
		UGameSettingCollectionPage* KeybindPage = NewObject<UGameSettingCollectionPage>();
		KeybindPage->SetDevName(TEXT("KeybindPage"));
		KeybindPage->SetDisplayName(LOCTEXT("KeybindPage_Name", "Keybinds"));
		KeybindPage->SetDescriptionRichText(LOCTEXT("KeybindPage_Description", "Configure the keybinds."));
		KeybindPage->SetNavigationText(LOCTEXT("KeybindPage_Navigation", "Edit"));

		KeybindPage->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

		Keybind->AddSetting(KeybindPage);

		static TSet<FName> AddedSettings;
		AddedSettings.Reset();

		//======================================
		//	キーバインド設定自動登録
		//======================================
		{
			const TArray<FLoadedMappableConfigPair>& RegisteredConfigs = InLocalPlayer->GetDeviceSettings()->GetAllRegisteredInputConfigs();
			const TMap<FName, FKey>& CustomKeyMap = InLocalPlayer->GetDeviceSettings()->GetCustomPlayerInputConfig();

			for (const FLoadedMappableConfigPair& InputConfigPair : RegisteredConfigs)
			{
				TArray<FEnhancedActionKeyMapping> ConfigMappings = InputConfigPair.Config->GetPlayerMappableKeys();
				if (ConfigMappings.IsEmpty())
				{
					UE_LOG(LogBEGameSettingRegistry, Warning, TEXT("PlayerMappableInputConfig '%s' has no player mappable keys in it! Skipping it in the setting registry..."), *InputConfigPair.Config->GetConfigName().ToString());
					continue;
				}

				UGameSettingCollection* ConfigSettingCollection = NewObject<UGameSettingCollection>();
				ConfigSettingCollection->SetDevName(InputConfigPair.Config->GetConfigName());
				ConfigSettingCollection->SetDisplayName(InputConfigPair.Config->GetDisplayName());
				KeybindPage->AddSetting(ConfigSettingCollection);

				// Add each player mappable key to the settings screen!
				for (FEnhancedActionKeyMapping& Mapping : ConfigMappings)
				{
					UBESettingValueKeyConfig* ExistingSetting = nullptr;

					// Make sure that we cannot add two settings with the same FName for saving purposes
					if (AddedSettings.Contains(Mapping.PlayerMappableOptions.Name))
					{
						UE_LOG(LogBEGameSettingRegistry, Warning, TEXT("A setting with the name '%s' from config '%s' has already been added! Please remove duplicate name."), *Mapping.PlayerMappableOptions.Name.ToString(), *InputConfigPair.Config->GetConfigName().ToString());
						continue;
					}

					for (UGameSetting* Setting : ConfigSettingCollection->GetChildSettings())
					{
						UBESettingValueKeyConfig* BEKeyboardSetting = Cast<UBESettingValueKeyConfig>(Setting);
						if (BEKeyboardSetting->GetSettingDisplayName().EqualToCaseIgnored(Mapping.PlayerMappableOptions.DisplayName))
						{
							ExistingSetting = BEKeyboardSetting;
							break;
						}
					}

					FEnhancedActionKeyMapping MappingSynthesized(Mapping);
					// If the player has bound a custom key to this action, then set it to that
					if (const FKey* PlayerBoundKey = CustomKeyMap.Find(Mapping.PlayerMappableOptions.Name))
					{
						MappingSynthesized.Key = *PlayerBoundKey;
					}

					if (MappingSynthesized.PlayerMappableOptions.Name != NAME_None && !MappingSynthesized.PlayerMappableOptions.DisplayName.IsEmpty())
					{
						UBESettingValueKeyConfig* InputBinding = nullptr;

						if (ExistingSetting)
						{
							InputBinding = ExistingSetting;

							InputBinding->SetInputData(MappingSynthesized, InputConfigPair.Config, ExistingSetting->GetMappableOptions().Num());
						}
						else
						{
							InputBinding = NewObject<UBESettingValueKeyConfig>();

							InputBinding->SetInputData(MappingSynthesized, InputConfigPair.Config, 0);

							ConfigSettingCollection->AddSetting(InputBinding);
						}

						AddedSettings.Add(MappingSynthesized.PlayerMappableOptions.Name);
					}
					else
					{
						UE_LOG(LogBEGameSettingRegistry, Warning, TEXT("A setting with the name '%s' from config '%s' could not be added, one of its names is empty!"), *Mapping.PlayerMappableOptions.Name.ToString(), *InputConfigPair.Config->GetConfigName().ToString());
						ensure(false);
					}
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
