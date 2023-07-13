// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameSettingRegistry.h"

#include "GameSetting/BEGameDeviceSettings.h" // IWYU pragma: keep

#include "DataSource/GameSettingDataSourceDynamic.h" // IWYU pragma: keep
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameSettingRegistry.generated.h"

class UObject;
class ULocalPlayer;
class UBELocalPlayer;
class UGameSettingCollection;

////////////////////////////////////////////////////////////////

DECLARE_LOG_CATEGORY_EXTERN(LogBEGameSettingRegistry, Log, Log);

#define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(UBELocalPlayer, GetSharedSettings),				\
		GET_FUNCTION_NAME_STRING_CHECKED(UBEGameSharedSettings, FunctionOrPropertyName)		\
	}))

#define GET_DEVICE_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(UBELocalPlayer, GetDeviceSettings),				\
		GET_FUNCTION_NAME_STRING_CHECKED(UBEGameDeviceSettings, FunctionOrPropertyName)		\
	}))

////////////////////////////////////////////////////////////////

/**
 * UBEGameSettingRegistry
 */
UCLASS()
class BECORE_API UBEGameSettingRegistry : public UGameSettingRegistry
{
	GENERATED_BODY()
public:
	UBEGameSettingRegistry() {}

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;

public:
	/**
	 * 設定を保存する
	 */
	virtual void SaveChanges() override;


protected:
	UPROPERTY() UGameSettingCollection* ControlSettings;
	UPROPERTY() UGameSettingCollection* VideoSettings;
	UPROPERTY() UGameSettingCollection* GameplaySettings;
	UPROPERTY() UGameSettingCollection* AudioSettings;

protected:
	/**
	 * コントロール設定カテゴリー
	 */
	virtual UGameSettingCollection* InitializeControlSettings(UBELocalPlayer* InLocalPlayer);
	virtual void AddKeyBindSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);
	virtual void AddKeyboardMouseSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);
	virtual void AddGamepadSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);
	virtual void AddControllerSharedSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);


	/**
	 * ビデオ設定カテゴリー
	 */
	virtual UGameSettingCollection* InitializeVideoSettings(UBELocalPlayer* InLocalPlayer);


	/**
	 * ゲームプレイ設定カテゴリー
	 */
	virtual UGameSettingCollection* InitializeGameplaySettings(UBELocalPlayer* InLocalPlayer);
	virtual void AddAccessibilitySettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);
	virtual void AddPrefStatSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);


	/**
	 * オーディオ設定カテゴリー
	 */
	virtual UGameSettingCollection* InitializeAudioSettings(UBELocalPlayer* InLocalPlayer);
};
