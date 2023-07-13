// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameFramework/GameUserSettings.h"

#include "Input/BEMappableConfigPair.h"
#include "Performance/BEPerformanceStatTypes.h"

#include "CommonInputBaseTypes.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "InputCoreTypes.h"
#include "Scalability.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BEGameDeviceSettings.generated.h"

class UBELocalPlayer;
class UObject;
class UPlayerMappableInputConfig;
class USoundControlBus;
class USoundControlBusMix;
struct FFrame;


/**
 * EBEAllowBackgroundAudioSetting
 * 
 *  バックグラウンドサウンドタイプ
 */
UENUM(BlueprintType)
enum class EBEAllowBackgroundAudioSetting : uint8
{
	Off,
	AllSounds,

	Num UMETA(Hidden)
};


/**
 * FBEScalabilitySnapshot
 * 
 *  グラフィック全体のクオリティレベルを保存する
 */
USTRUCT()
struct FBEScalabilitySnapshot
{
	GENERATED_BODY()

	FBEScalabilitySnapshot();

	Scalability::FQualityLevels Qualities;
	bool bActive = false;
	bool bHasOverrides = false;
};


/**
 * UBEGameDeviceSettings
 * 
 *  このゲームをプレイするデバイスに依存する設定を保存するためのクラス。
 *  このクラスで保存される情報は基本的にクラウドセーブなどで共有しない。
 *  
 *  このクラスで保存するデータは以下である。
 *  - ディスプレイ設定
 *  - 画質設定
 *  - フレームレート設定
 *  - 統計情報設定
 *  - オーディオデバイス設定
 *  - コントローラー設定
 *  - キーバインド設定
 */
UCLASS()
class UBEGameDeviceSettings final : public UGameUserSettings
{
	GENERATED_BODY()
public:
	UBEGameDeviceSettings();

public:
	//~UObject interface
	virtual void BeginDestroy() override;
	//~End of UObject interface

	//~UGameUserSettings interface
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;

	virtual void ResetToCurrentSettings() override;
	virtual void ApplyNonResolutionSettings() override;
	//~End of UGameUserSettings interface

	void OnExperienceLoaded();
	void OnHotfixDeviceProfileApplied();


	// =========================================
	// ディスプレイ設定
	// =========================================
protected:
	/**
	 * ApplySafeZoneScale
	 *
	 *  ディスプレイの表示可能な SafeZone を適応する
	 */
	void ApplySafeZoneScale();

private:
	// ディスプレイの表示可能な SafeZone
	UPROPERTY(Config) float SafeZoneScale = -1;

public:
	UFUNCTION() bool IsSafeZoneSet() const { return SafeZoneScale != -1; }
	UFUNCTION() float GetSafeZone() const { return SafeZoneScale >= 0 ? SafeZoneScale : 0; }
	UFUNCTION() void SetSafeZone(float Value);


protected:
	/**
	 * ApplyDisplayGamma
	 *
	 *  ディスプレイの明るさを適応
	 */
	void ApplyDisplayGamma();

private:
	// ディスプレイの明るさ
	UPROPERTY(Config) float DisplayGamma = 2.2;

public:
	UFUNCTION() float GetDisplayGamma() const { return DisplayGamma; }
	UFUNCTION() void SetDisplayGamma(float InGamma);


	// =========================================
	// 画質設定
	// =========================================
public:
	/**
	 * ApplyScalabilitySettings
	 *
	 *  グラフィックの全体クオリティを適応
	 */
	void ApplyScalabilitySettings();

	//~UGameUserSettings interface
	virtual int32 GetOverallScalabilityLevel() const override;
	virtual void SetOverallScalabilityLevel(int32 Value) override;
	//~End of UGameUserSettings interface


public:
	/** Returns true if this platform can run the auto benchmark */
	UFUNCTION(BlueprintCallable, Category = "GameSettings")
	bool CanRunAutoBenchmark() const;

	/** Returns true if this user should run the auto benchmark as it has never been run */
	UFUNCTION(BlueprintCallable, Category = "GameSettings")
	bool ShouldRunAutoBenchmarkAtStartup() const;

	/** Run the auto benchmark, optionally saving right away */
	UFUNCTION(BlueprintCallable, Category = "GameSettings")
	void RunAutoBenchmark(bool bSaveImmediately);


private:
	// デバイスごとのクオリティプリセットを適応するための識別子
	UPROPERTY(Transient) FString DesiredUserChosenDeviceProfileSuffix;
	UPROPERTY(Transient) FString CurrentAppliedDeviceProfileOverrideSuffix;
	UPROPERTY(config)	 FString UserChosenDeviceProfileSuffix;

	FBEScalabilitySnapshot DeviceDefaultScalabilitySettings;
	bool bSettingOverallQualityGuard = false;

public:
	UFUNCTION() FString GetDesiredDeviceProfileQualitySuffix() const { return DesiredUserChosenDeviceProfileSuffix; }
	UFUNCTION() void SetDesiredDeviceProfileQualitySuffix(const FString& InDesiredSuffix) { DesiredUserChosenDeviceProfileSuffix = InDesiredSuffix; }


	/**
	 *  現在のデバイスでサポートされている全体クオリティレベルを取得
	 *
	 * @Return 最大クオリティレベル
	 */
public:
	int32 GetMaxSupportedOverallQualityLevel() const;


	// =========================================
	// フレームレート設定
	// =========================================
private:
	// 様々な状態におけるフレームレートの制限
	UPROPERTY(Config) float FrameRateLimit_OnBattery;
	UPROPERTY(Config) float FrameRateLimit_InMenu;
	UPROPERTY(Config) float FrameRateLimit_WhenBackgrounded;

public:
	UFUNCTION() float GetFrameRateLimit_OnBattery() const { return FrameRateLimit_OnBattery; }
	UFUNCTION() void SetFrameRateLimit_OnBattery(float NewLimitFPS);

	UFUNCTION() float GetFrameRateLimit_InMenu() const { return FrameRateLimit_InMenu; }
	UFUNCTION() void SetFrameRateLimit_InMenu(float NewLimitFPS);

	UFUNCTION() float GetFrameRateLimit_WhenBackgrounded() const { return FrameRateLimit_WhenBackgrounded; }
	UFUNCTION() void SetFrameRateLimit_WhenBackgrounded(float NewLimitFPS);

	UFUNCTION() float GetFrameRateLimit_Always() const;
	UFUNCTION() void SetFrameRateLimit_Always(float NewLimitFPS);

	//~UGameUserSettings interface
	virtual float GetEffectiveFrameRateLimit() override;
	//~End of UGameUserSettings interface

protected:
	void UpdateEffectiveFrameRateLimit();
	void UpdateGameModeDeviceProfileAndFps();
	void UpdateConsoleFramePacing();
	void UpdateDesktopFramePacing();
	void UpdateDynamicResFrameTime(float TargetFPS);


	/**
	 *  フロントエンド(メニューなど)向けの設定を使うかどうか設定
	 */
public:
	void SetShouldUseFrontendPerformanceSettings(bool bInFrontEnd);
protected:
	bool ShouldUseFrontendPerformanceSettings() const;
private:
	bool bInFrontEndForPerformancePurposes = false;


	// =========================================
	// 統計情報設定
	// =========================================
private:
	// 統計情報の表示リスト
	UPROPERTY(Config) TMap<EBEDisplayablePerformanceStat, EBEStatDisplayMode> DisplayStatList;

public:
	UFUNCTION() EBEStatDisplayMode GetPerfStatDisplayState(EBEDisplayablePerformanceStat Stat) const;
	UFUNCTION() void SetPerfStatDisplayState(EBEDisplayablePerformanceStat Stat, EBEStatDisplayMode DisplayMode);


	// =========================================
	// オーディオデバイス設定
	// =========================================
private:
	UPROPERTY(Transient) TMap<FName, USoundControlBus*> ControlBusMap;
	UPROPERTY(Transient) USoundControlBusMix* ControlBusMix = nullptr;
	UPROPERTY(Transient) bool bSoundControlBusMixLoaded;

public:
	/**
	 * LoadUserControlBusMix
	 *
	 *  サウンドのコントロールバスを読み込む
	 */
	void LoadUserControlBusMix();


private:
	// 音声出力デバイスのID
	UPROPERTY(Config) FString AudioOutputDeviceId;

public:
	UFUNCTION() FString GetAudioOutputDeviceId() const { return AudioOutputDeviceId; }
	UFUNCTION() void SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId);


public:
	// 立体音響の仕様を求めているかどうか
	UPROPERTY(Transient) bool bDesiredHeadphoneMode;

private:
	// 立体音響を利用するかどうか
	UPROPERTY(config) bool bUseHeadphoneMode;

public:
	UFUNCTION() bool IsHeadphoneModeEnabled() const { return bUseHeadphoneMode; }
	UFUNCTION() void SetHeadphoneModeEnabled(bool bEnabled);
	UFUNCTION() bool CanModifyHeadphoneModeEnabled() const;


private:
	// HDR オーディオを使用するかどうか
	UPROPERTY(config) bool bUseHDRAudioMode;

public:
	UFUNCTION() bool IsHDRAudioModeEnabled() const { return bUseHDRAudioMode; }
	UFUNCTION() void SetHDRAudioModeEnabled(bool bEnabled);


private:
	// バックグランドで音楽を再生し続けるかどうか
	UPROPERTY(config) EBEAllowBackgroundAudioSetting AllowAudioInBackground = EBEAllowBackgroundAudioSetting::Off;

public:
	UFUNCTION() EBEAllowBackgroundAudioSetting GetAllowAudioInBackgroundSetting() const { return AllowAudioInBackground; }
	UFUNCTION() void SetAllowAudioInBackgroundSetting(EBEAllowBackgroundAudioSetting NewValue);

	void ApplyAudioSettings();


private:
	// それぞれのサウンドボリューム
	UPROPERTY(config) float OverallVolume	= 1.0f;
	UPROPERTY(config) float MusicVolume		= 1.0f;
	UPROPERTY(config) float SoundFXVolume	= 1.0f;
	UPROPERTY(config) float DialogueVolume	= 1.0f;
	UPROPERTY(config) float VoiceChatVolume = 1.0f;

public:
	UFUNCTION() float GetOverallVolume() const { return OverallVolume; }
	UFUNCTION() void SetOverallVolume(float InVolume);

	UFUNCTION() float GetMusicVolume() const { return MusicVolume; }
	UFUNCTION() void SetMusicVolume(float InVolume);

	UFUNCTION() float GetSoundFXVolume() const { return SoundFXVolume; }
	UFUNCTION() void SetSoundFXVolume(float InVolume);

	UFUNCTION() float GetDialogueVolume() const { return DialogueVolume; }
	UFUNCTION() void SetDialogueVolume(float InVolume);

	UFUNCTION() float GetVoiceChatVolume() const { return VoiceChatVolume; }
	UFUNCTION() void SetVoiceChatVolume(float InVolume);

protected:
	/**
	 * SetVolumeForControlBus
	 * 
	 *  ControlBusの音量を変える
	 *
	 * @Param InSoundControlBus	音量を変えるControlBus
	 * @Param InVolume			新しい音量
	 */
	void SetVolumeForControlBus(USoundControlBus* InSoundControlBus, float InVolume);



	// =========================================
	// コントローラー設定
	// =========================================
private:
	// コントローラーのプラットフォーム(PS, XBOXなど)
	UPROPERTY(config) FName ControllerPlatform;
	UPROPERTY(config) FName ControllerPreset = TEXT("Default");

public:
	UFUNCTION() void SetControllerPlatform(const FName InControllerPlatform);
	UFUNCTION() FName GetControllerPlatform() const { return ControllerPlatform; }


	// =========================================
	// キーバインド設定
	// =========================================
private:
	UPROPERTY(VisibleAnywhere) TArray<FLoadedMappableConfigPair> RegisteredInputConfigs;
	UPROPERTY(Config) FName InputConfigName = TEXT("Default");
	UPROPERTY(Config) TMap<FName, FKey> CustomKeyboardConfig;

public:
	const TArray<FLoadedMappableConfigPair>& GetAllRegisteredInputConfigs() const { return RegisteredInputConfigs; }
	const TMap<FName, FKey>& GetCustomPlayerInputConfig() const { return CustomKeyboardConfig; }


public:
	/** Register the given input config with the settings to make it available to the player. */
	void RegisterInputConfig(ECommonInputType Type, const UPlayerMappableInputConfig* NewConfig, const bool bIsActive);

	/** Unregister the given input config. Returns the number of configs removed. */
	int32 UnregisterInputConfig(const UPlayerMappableInputConfig* ConfigToRemove);

	/**
	 * Get all registered input configs that match the input type.
	 *
	 * @param Type		The type of config to get, ECommonInputType::Count will include all configs.
	 * @param OutArray	Array to be populated with the current registered input configs that match the type
	 */
	void GetRegisteredInputConfigsOfType(ECommonInputType Type, OUT TArray<FLoadedMappableConfigPair>& OutArray) const;

	/**
	 * Returns the display name of any actions with that key bound to it
	 *
	 * @param InKey The key to check for current mappings of
	 * @param OutActionNames Array to store display names of actions of bound keys
	 */
	void GetAllMappingNamesFromKey(const FKey InKey, TArray<FName>& OutActionNames);

	/**
	 * Maps the given keyboard setting to the new key
	 *
	 * @param MappingName	The name of the FPlayerMappableKeyOptions that you would like to change
	 * @param NewKey		The new key to bind this option to
	 * @param LocalPlayer   local player to reset the keybinding on
	 */
	void AddOrUpdateCustomKeyboardBindings(const FName MappingName, const FKey NewKey, UBELocalPlayer* LocalPlayer);

	/**
	 * Resets keybinding to its default value in its input mapping context
	 *
	 * @param MappingName	The name of the FPlayerMappableKeyOptions that you would like to change
	 * @param LocalPlayer   local player to reset the keybinding on
	 */
	void ResetKeybindingToDefault(const FName MappingName, UBELocalPlayer* LocalPlayer);

	/** Resets all keybindings to their default value in their input mapping context
	 * @param LocalPlayer   local player to reset the keybinding on
	 */
	void ResetKeybindingsToDefault(UBELocalPlayer* LocalPlayer);


public:
	/**
	 * 名前から InputConfig を取得する。構成が存在しない場合は、nullptr が返される。
	 *
	 * @Param ConfigName	名前
	 * @Return 				InputConfig
	 */
	UFUNCTION(BlueprintCallable, Category = "GameSettings")
	const UPlayerMappableInputConfig* GetInputConfigByName(FName ConfigName) const;


	//======================================
	//	イベント
	//======================================
private:
	/**
	 *  アプリの状態が変わったとき
	 */
	void OnAppActivationStateChanged(bool bIsActive);
	void ReapplyThingsDueToPossibleDeviceProfileChange();

	FDelegateHandle OnApplicationActivationStateChangedHandle;

	/**
	 *  パフォーマンス表示設定が変わったとき
	 */
public:
	DECLARE_EVENT(UBEGameDeviceSettings, FPerfStatSettingsChanged);
	FPerfStatSettingsChanged OnPerfStatSettingsChangedEvent;

	/**
	 *  音響出力デバイスが変更されたとき
	 */
public:
	DECLARE_EVENT_OneParam(UBEGameDeviceSettings, FAudioDeviceChanged, const FString& /*DeviceId*/);
	FAudioDeviceChanged OnAudioOutputDeviceChanged;

	/**
	 *  キーコンフィグの状態が変わったとき
	 */
public:
	DECLARE_EVENT_OneParam(UBEGameDeviceSettings, FInputConfigDelegate, const FLoadedMappableConfigPair& /*Config*/);

	/** Delegate called when a new input config has been registered */
	FInputConfigDelegate OnInputConfigRegistered;

	/** Delegate called when a registered input config has been activated */
	FInputConfigDelegate OnInputConfigActivated;

	/** Delegate called when a registered input config has been deactivate */
	FInputConfigDelegate OnInputConfigDeactivated;


	//======================================
	//	ユーティリティ
	//======================================
public:
	/**
	 *  このクラスのインスタンスが存在する場合にそれ取得する
	 *
	 * @Return 設定のインスタンス
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameSettings", meta = (DisplayName = "GetBELocalSettings"))
	static UBEGameDeviceSettings* Get();
};
