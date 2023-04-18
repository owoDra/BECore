// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

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

#include "BESettingsLocal.generated.h"

class UBELocalPlayer;
class UObject;
class UPlayerMappableInputConfig;
class USoundControlBus;
class USoundControlBusMix;
struct FFrame;

////////////////////////////////////////
//	構造体

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


////////////////////////////////////////
//	クラス

/**
 * UBESettingsLocal
 * 
 *  このプロジェクトでユーザーのゲーム設定を記録するためのクラスの一つ。
 *  主に、ゲームをプレイする際のハードウェアに関係するような、違うデバイス間で
 *  共有されたくない設定を保存する。
 */
UCLASS()
class UBESettingsLocal : public UGameUserSettings
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBESettingsLocal();

	//~UGameUserSettings interface
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;
	//~End of UGameUserSettings interface

	//~UObject interface
	virtual void BeginDestroy() override;
	//~End of UObject interface

	void OnExperienceLoaded();
	void OnHotfixDeviceProfileApplied();

	void LoadUserControlBusMix();

private:
	UPROPERTY(Transient)
		TMap<FName/*SoundClassName*/, USoundControlBus*> ControlBusMap;

	UPROPERTY(Transient)
		USoundControlBusMix* ControlBusMix = nullptr;

	UPROPERTY(Transient)
		bool bSoundControlBusMixLoaded;

	FBEScalabilitySnapshot DeviceDefaultScalabilitySettings;

	bool bSettingOverallQualityGuard = false;


	//======================================
	//	データ
	//======================================

	////////////////////////////////////////
	// ゲームプレイ設定

		/* ===== パフォーマンスの表示設定 ===== */
public:
		UFUNCTION(BlueprintCallable, Category = "GameSettings")
			EBEStatDisplayMode GetPerfStatDisplayState(EBEDisplayablePerformanceStat Stat) const;
		UFUNCTION(BlueprintCallable, Category = "GameSettings")
			void SetPerfStatDisplayState(EBEDisplayablePerformanceStat Stat, EBEStatDisplayMode DisplayMode);
private:
		UPROPERTY(Config)
			TMap<EBEDisplayablePerformanceStat, EBEStatDisplayMode> DisplayStatList;

	// ゲームプレイ設定
	////////////////////////////////////////


	////////////////////////////////////////
	// コントロール設定

		/* ===== コントローラープラットフォーム設定 ===== */
		// Sets the controller representation to use, a single platform might support multiple kinds of controllers.  For
		// example, Win64 games could be played with both an XBox or Playstation controller.
public:
		UFUNCTION()
			void SetControllerPlatform(const FName InControllerPlatform);
		UFUNCTION()
			FName GetControllerPlatform() const { return ControllerPlatform; }
private:
		UPROPERTY(config)
			FName ControllerPlatform;
		UPROPERTY(config)
			FName ControllerPreset = TEXT("Default");


		/* ===== キーバインド設定 ===== */
public:
		const TArray<FLoadedMappableConfigPair>& GetAllRegisteredInputConfigs() const { return RegisteredInputConfigs; }
		const TMap<FName, FKey>& GetCustomPlayerInputConfig() const { return CustomKeyboardConfig; }
private:
		UPROPERTY(VisibleAnywhere)
			TArray<FLoadedMappableConfigPair> RegisteredInputConfigs;
		UPROPERTY(Config)
			FName InputConfigName = TEXT("Default");
		UPROPERTY(Config)
			TMap<FName, FKey> CustomKeyboardConfig;

	// コントロール設定
	////////////////////////////////////////


	////////////////////////////////////////
	// ビデオ設定

		/* ===== セーフゾーン設定 ===== */
public:
		UFUNCTION()
			bool IsSafeZoneSet() const { return SafeZoneScale != -1; }
		UFUNCTION()
			float GetSafeZone() const { return SafeZoneScale >= 0 ? SafeZoneScale : 0; }
		UFUNCTION()
			void SetSafeZone(float Value);
protected:
			void ApplySafeZoneScale();
private:
		UPROPERTY(Config)
			float SafeZoneScale = -1;


		/* ===== 明るさ設定 ===== */
public:
		UFUNCTION(BlueprintCallable, Category = "GameSettings")
			float GetDisplayGamma() const { return DisplayGamma; }
		UFUNCTION(BlueprintCallable, Category = "GameSettings")
			void SetDisplayGamma(float InGamma);
protected:
			void ApplyDisplayGamma();
private:
		UPROPERTY(Config)
			float DisplayGamma = 2.2;


		/* ===== 全体クオリティ設定 ===== */
public:
		UFUNCTION()
			FString GetDesiredDeviceProfileQualitySuffix() const { return DesiredUserChosenDeviceProfileSuffix; }
		UFUNCTION()
			void SetDesiredDeviceProfileQualitySuffix(const FString& InDesiredSuffix) { DesiredUserChosenDeviceProfileSuffix = InDesiredSuffix; }

		//~UGameUserSettings interface
		virtual int32 GetOverallScalabilityLevel() const override;
		virtual void SetOverallScalabilityLevel(int32 Value) override;
		//~End of UGameUserSettings interface

		void ApplyScalabilitySettings();
private:
		UPROPERTY(Transient)
			FString DesiredUserChosenDeviceProfileSuffix;
		UPROPERTY(Transient)
			FString CurrentAppliedDeviceProfileOverrideSuffix;
		UPROPERTY(config)
			FString UserChosenDeviceProfileSuffix;


		/* ===== フレームレート設定 ===== */
public:
		UFUNCTION()
			float GetFrameRateLimit_OnBattery() const { return FrameRateLimit_OnBattery; }
		UFUNCTION()
			void SetFrameRateLimit_OnBattery(float NewLimitFPS);

		UFUNCTION()
			float GetFrameRateLimit_InMenu() const { return FrameRateLimit_InMenu; }
		UFUNCTION()
			void SetFrameRateLimit_InMenu(float NewLimitFPS);

		UFUNCTION()
			float GetFrameRateLimit_WhenBackgrounded() const { return FrameRateLimit_WhenBackgrounded; }
		UFUNCTION()
			void SetFrameRateLimit_WhenBackgrounded(float NewLimitFPS);

		UFUNCTION()
			float GetFrameRateLimit_Always() const;
		UFUNCTION()
			void SetFrameRateLimit_Always(float NewLimitFPS);
private:
		UPROPERTY(Config)
			float FrameRateLimit_OnBattery;
		UPROPERTY(Config)
			float FrameRateLimit_InMenu;
		UPROPERTY(Config)
			float FrameRateLimit_WhenBackgrounded;

	// ビデオ設定
	////////////////////////////////////////
	

	////////////////////////////////////////
	// オーディオ設定

		/* ===== 音響出力デバイス設定 ===== */
public:
		UFUNCTION()
			FString GetAudioOutputDeviceId() const { return AudioOutputDeviceId; }
		UFUNCTION()
			void SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId);
private:
		UPROPERTY(Config)
			FString AudioOutputDeviceId;


		/* ===== 音量設定 ===== */
public:
		UFUNCTION()
			float GetOverallVolume() const { return OverallVolume; }
		UFUNCTION()
			void SetOverallVolume(float InVolume);

		UFUNCTION()
			float GetMusicVolume() const { return MusicVolume; }
		UFUNCTION()
			void SetMusicVolume(float InVolume);

		UFUNCTION()
			float GetSoundFXVolume() const { return SoundFXVolume; }
		UFUNCTION()
			void SetSoundFXVolume(float InVolume);

		UFUNCTION()
			float GetDialogueVolume() const { return DialogueVolume; }
		UFUNCTION()
			void SetDialogueVolume(float InVolume);

		UFUNCTION()
			float GetVoiceChatVolume() const { return VoiceChatVolume; }
		UFUNCTION()
			void SetVoiceChatVolume(float InVolume);
private:
		UPROPERTY(config)
			float OverallVolume = 1.0f;
		UPROPERTY(config)
			float MusicVolume = 1.0f;
		UPROPERTY(config)
			float SoundFXVolume = 1.0f;
		UPROPERTY(config)
			float DialogueVolume = 1.0f;
		UPROPERTY(config)
			float VoiceChatVolume = 1.0f;


		/* ===== 立体音響設定 ===== */
public:
		UFUNCTION()
			bool IsHeadphoneModeEnabled() const { return bUseHeadphoneMode; }
		UFUNCTION()
			void SetHeadphoneModeEnabled(bool bEnabled);
		UFUNCTION()
			bool CanModifyHeadphoneModeEnabled() const;
public:
		UPROPERTY(Transient)
			bool bDesiredHeadphoneMode;
private:
		UPROPERTY(config)
			bool bUseHeadphoneMode;


		/* ===== HDRオーディオ設定 ===== */
public:
		UFUNCTION()
			bool IsHDRAudioModeEnabled() const { return bUseHDRAudioMode; }
		UFUNCTION()
			void SetHDRAudioModeEnabled(bool bEnabled);
private:
		UPROPERTY(config)
			bool bUseHDRAudioMode;

	// オーディオ設定
	////////////////////////////////////////


	//======================================
	//	操作
	//======================================
public:
	//~UGameUserSettings interface
	virtual void ResetToCurrentSettings() override;
	virtual void ApplyNonResolutionSettings() override;
	//~End of UGameUserSettings interface


	/* ===== フレームレート ===== */
protected:
	void UpdateEffectiveFrameRateLimit();
	void UpdateGameModeDeviceProfileAndFps();
	void UpdateConsoleFramePacing();
	void UpdateDesktopFramePacing();
	void UpdateDynamicResFrameTime(float TargetFPS);


	/* ===== ベンチマーク ===== */
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


	/* ===== キーバインド ===== */
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


	//======================================
	//	ユーティリティ
	//======================================

	/**
	 *  このクラスのインスタンスが存在する場合にそれ取得する
	 * 
	 * @Return 設定のインスタンス
	 */
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameSettings", meta = (DisplayName = "GetBELocalSettings"))
	static UBESettingsLocal* Get();

	/**
	 *  フロントエンド(メニューなど)向けの設定を使うかどうか設定
	 */
public:
	void SetShouldUseFrontendPerformanceSettings(bool bInFrontEnd);
protected:
	bool ShouldUseFrontendPerformanceSettings() const;
private:
	bool bInFrontEndForPerformancePurposes = false;

	/**
	 *  現在のデバイスでサポートされている全体クオリティレベルを取得
	 * 
	 * @Return 最大クオリティレベル
	 */
public:
	int32 GetMaxSupportedOverallQualityLevel() const;

	/**
	 *  ControlBusの音量を変える
	 *
	 * @Param InSoundControlBus	音量を変えるControlBus
	 * @Param InVolume			新しい音量
	 */
protected:
	void SetVolumeForControlBus(USoundControlBus* InSoundControlBus, float InVolume);

	/**
	 * 名前から InputConfig を取得する。構成が存在しない場合は、nullptr が返される。
	 *
	 * @Param ConfigName	名前
	 * @Return 				InputConfig
	 */
public:
	UFUNCTION(BlueprintCallable, Category = "GameSettings")
		const UPlayerMappableInputConfig* GetInputConfigByName(FName ConfigName) const;

public:
	//~UGameUserSettings interface
	virtual float GetEffectiveFrameRateLimit() override;
	//~End of UGameUserSettings interface

	//======================================
	//	イベント
	//======================================

	/**
	 *  アプリの状態が変わったとき
	 */
private:
	void OnAppActivationStateChanged(bool bIsActive);
	void ReapplyThingsDueToPossibleDeviceProfileChange();

	FDelegateHandle OnApplicationActivationStateChangedHandle;

	/**
	 *  パフォーマンス表示設定が変わったとき
	 */
public:
	DECLARE_EVENT(UBESettingsLocal, FPerfStatSettingsChanged);
	FPerfStatSettingsChanged OnPerfStatSettingsChangedEvent;

	/**
	 *  音響出力デバイスが変更されたとき
	 */
public:
	DECLARE_EVENT_OneParam(UBESettingsLocal, FAudioDeviceChanged, const FString& /*DeviceId*/);
	FAudioDeviceChanged OnAudioOutputDeviceChanged;

	/**
	 *  キーコンフィグの状態が変わったとき
	 */
public:
	DECLARE_EVENT_OneParam(UBESettingsLocal, FInputConfigDelegate, const FLoadedMappableConfigPair& /*Config*/);

	/** Delegate called when a new input config has been registered */
	FInputConfigDelegate OnInputConfigRegistered;

	/** Delegate called when a registered input config has been activated */
	FInputConfigDelegate OnInputConfigActivated;

	/** Delegate called when a registered input config has been deactivate */
	FInputConfigDelegate OnInputConfigDeactivated;
};
