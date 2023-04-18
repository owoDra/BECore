// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "GameSettingRegistry.h"

#include "Settings/BESettingsLocal.h" // IWYU pragma: keep

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
		GET_FUNCTION_NAME_STRING_CHECKED(UBESettingsShared, FunctionOrPropertyName)			\
	}))

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
		GET_FUNCTION_NAME_STRING_CHECKED(UBELocalPlayer, GetLocalSettings),					\
		GET_FUNCTION_NAME_STRING_CHECKED(UBESettingsLocal, FunctionOrPropertyName)			\
	}))

////////////////////////////////////////////////////////////////

/**
 * UBEGameSettingRegistry
 */
UCLASS()
class UBEGameSettingRegistry : public UGameSettingRegistry
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBEGameSettingRegistry();

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;

protected:
	UPROPERTY()
		UGameSettingCollection* ControlSettings;

	UPROPERTY()
		UGameSettingCollection* VideoSettings;

	UPROPERTY()
		UGameSettingCollection* GameplaySettings;

	UPROPERTY()
		UGameSettingCollection* AudioSettings;


	//======================================
	//	設定コレクション定義
	//======================================
protected:
	/**
	 * コントロール設定カテゴリー
	 */
	UGameSettingCollection* InitializeControlSettings(UBELocalPlayer* InLocalPlayer);
	void AddKeyBindSettingsPage(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);


	/**
	 * ビデオ設定カテゴリー
	 */
	UGameSettingCollection* InitializeVideoSettings(UBELocalPlayer* InLocalPlayer);


	/**
	 * ゲームプレイ設定カテゴリー
	 */
	UGameSettingCollection* InitializeGameplaySettings(UBELocalPlayer* InLocalPlayer);
	void AddSubtitleSettingsPage(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);
	void AddReticleSettingsPage(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);
	void AddPrefStatSettingsPage(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer);


	/**
	 * オーディオ設定カテゴリー
	 */
	UGameSettingCollection* InitializeAudioSettings(UBELocalPlayer* InLocalPlayer);


	//======================================
	//	ユーティリティ
	//======================================
public:
	static UBEGameSettingRegistry* Get(UBELocalPlayer* InLocalPlayer);

	virtual void SaveChanges() override;
};
