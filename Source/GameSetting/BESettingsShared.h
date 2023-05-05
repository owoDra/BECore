// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "SubtitleDisplaySubsystem.h"
#include "GameFramework/SaveGame.h"
#include "BESettingsShared.generated.h"

class ULocalPlayer;
class UBELocalPlayer;

////////////////////////////////////////
//	列挙型

/**
 *  色覚特性タイプ
 */
UENUM(BlueprintType)
enum class EColorBlindMode : uint8
{
	Off,
	// Deuteranope (green weak/blind)
	Deuteranope,
	// Protanope (red weak/blind)
	Protanope,
	// Tritanope(blue weak / bind)
	Tritanope
};

/**
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
 *  レティクルスタイル
 */
UENUM(BlueprintType)
enum class EReticleStyle : uint8
{
	Default,	// カスタムレティクルを使用しない
	Auto,		// 自動でカスタムレティクルを切り替える
	Custom,		// 全てカスタムレティクルを使用する

	Num UMETA(Hidden)
};

/**
 *  ゲームパッド感度タイプ
 */
UENUM(BlueprintType)
enum class EBEGamepadSensitivity : uint8
{
	Invalid = 0		UMETA(Hidden),

	Slow			UMETA(DisplayName = "01 - Slow"),
	SlowPlus		UMETA(DisplayName = "02 - Slow+"),
	SlowPlusPlus	UMETA(DisplayName = "03 - Slow++"),
	Normal			UMETA(DisplayName = "04 - Normal"),
	NormalPlus		UMETA(DisplayName = "05 - Normal+"),
	NormalPlusPlus	UMETA(DisplayName = "06 - Normal++"),
	Fast			UMETA(DisplayName = "07 - Fast"),
	FastPlus		UMETA(DisplayName = "08 - Fast+"),
	FastPlusPlus	UMETA(DisplayName = "09 - Fast++"),
	Insane			UMETA(DisplayName = "10 - Insane"),

	MAX				UMETA(Hidden),
};


////////////////////////////////////////
//	構造体

/**
 * FReticle
 *
 *  グラフィック全体のクオリティレベルを保存する
 */
USTRUCT(BlueprintType)
struct FReticle
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		FLinearColor DotColor = FLinearColor(1.0, 1.0, 1.0, 1.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		FLinearColor InlineColor = FLinearColor(1.0, 1.0, 1.0, 1.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		FLinearColor OutlineColor = FLinearColor(0.0, 0.0, 0.0, 0.25);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		bool bShowCenterDot = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		double DotSize = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		double OutlineThickness = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		double LineThicknessHori = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		double LineLengthHori = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		double LineOffsetHori = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		double LineThicknessVert = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		double LineLengthVert = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reticle")
		double LineOffsetVert = 8;
};


//======================================
//	クラス
//======================================

/**
 * UBESettingsShared 
 * 
 *  このプロジェクトでユーザーのゲーム設定を記録するためのクラスの一つ。
 *  主に、ゲームをプレイする際、マウス感度などユーザーのプレイスタイルに関わるような、
 *  違うデバイス間で共有されてもよい設定を保存する。
 */
UCLASS()
class UBESettingsShared : public USaveGame
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBESettingsShared();

	void Initialize(ULocalPlayer* LocalPlayer);

private:
	UPROPERTY(Transient)
		UBELocalPlayer* OwningPlayer = nullptr;


	//======================================
	//	データ
	//======================================
	
	////////////////////////////////////////
	// ゲームプレイ設定

	// ===== 言語設定 =====
public:
	/** Gets the pending culture */
	const FString& GetPendingCulture() const;

	/** Sets the pending culture to apply */
	void SetPendingCulture(const FString& NewCulture);

	// Called when the culture changes.
	void OnCultureChanged();

	/** Clears the pending culture to apply */
	void ClearPendingCulture();

	bool IsUsingDefaultCulture() const;

	void ResetToDefaultCulture();
	bool ShouldResetToDefaultCulture() const { return bResetToDefaultCulture; }

	void ApplyCultureSettings();
	void ResetCultureToCurrentSettings();

private:
	/** The pending culture to apply */
	UPROPERTY(Transient)
		FString PendingCulture;

	/* If true, resets the culture to default. */
	bool bResetToDefaultCulture = false;


	// ===== 字幕設定 =====
public:
	UFUNCTION()
		bool GetSubtitlesEnabled() const { return bEnableSubtitles; }
	UFUNCTION()
		void SetSubtitlesEnabled(bool Value) { ChangeValueAndDirty(bEnableSubtitles, Value); }

	UFUNCTION()
		ESubtitleDisplayTextSize GetSubtitlesTextSize() const { return SubtitleTextSize; }
	UFUNCTION()
		void SetSubtitlesTextSize(ESubtitleDisplayTextSize Value) { ChangeValueAndDirty(SubtitleTextSize, Value); ApplySubtitleOptions(); }

	UFUNCTION()
		ESubtitleDisplayTextColor GetSubtitlesTextColor() const { return SubtitleTextColor; }
	UFUNCTION()
		void SetSubtitlesTextColor(ESubtitleDisplayTextColor Value) { ChangeValueAndDirty(SubtitleTextColor, Value); ApplySubtitleOptions(); }

	UFUNCTION()
		ESubtitleDisplayTextBorder GetSubtitlesTextBorder() const { return SubtitleTextBorder; }
	UFUNCTION()
		void SetSubtitlesTextBorder(ESubtitleDisplayTextBorder Value) { ChangeValueAndDirty(SubtitleTextBorder, Value); ApplySubtitleOptions(); }

	UFUNCTION()
		ESubtitleDisplayBackgroundOpacity GetSubtitlesBackgroundOpacity() const { return SubtitleBackgroundOpacity; }
	UFUNCTION()
		void SetSubtitlesBackgroundOpacity(ESubtitleDisplayBackgroundOpacity Value) { ChangeValueAndDirty(SubtitleBackgroundOpacity, Value); ApplySubtitleOptions(); }

	void ApplySubtitleOptions();

private:
	UPROPERTY()
		bool bEnableSubtitles = true;

	UPROPERTY()
		ESubtitleDisplayTextSize SubtitleTextSize = ESubtitleDisplayTextSize::Medium;

	UPROPERTY()
		ESubtitleDisplayTextColor SubtitleTextColor = ESubtitleDisplayTextColor::White;

	UPROPERTY()
		ESubtitleDisplayTextBorder SubtitleTextBorder = ESubtitleDisplayTextBorder::None;

	UPROPERTY()
		ESubtitleDisplayBackgroundOpacity SubtitleBackgroundOpacity = ESubtitleDisplayBackgroundOpacity::Medium;


	// ===== レティクル編集 =====
public:
	UFUNCTION()
		bool ShouldOverrideTargetingReticleColor() const { return bOverrideTargetingReticleColor; }
	UFUNCTION()
		void SetOverrideTargetingReticleColor(bool NewValue) { bOverrideTargetingReticleColor = NewValue; }

	UFUNCTION()
		FLinearColor GetTargetingReticleColor() const { return TargetingReticleColor; }
	UFUNCTION()
		void SetTargetingReticleColor(FLinearColor NewValue) { TargetingReticleColor = NewValue; }

	UFUNCTION(BlueprintCallable)
		EReticleStyle GetReticleStyle() const { return ReticleStyle; }
	UFUNCTION()
		void SetReticleStyle(EReticleStyle NewValue) { ReticleStyle = NewValue; }

	UFUNCTION(BlueprintCallable)
		FReticle GetCurrentReticle() const { return ReticleProfiles[CurrentReticleIdx]; }
	UFUNCTION()
		void SetCurrentReticle(FReticle NewValue) { ReticleProfiles[CurrentReticleIdx] = NewValue; }

	UFUNCTION()
		int32 GetReticleIndex() const { return CurrentReticleIdx; }
	UFUNCTION()
		void SetReticleIndex(int32 NewValue) { CurrentReticleIdx = NewValue; }

	UFUNCTION()
		FLinearColor GetDotColor() const { return ReticleProfiles[CurrentReticleIdx].DotColor; }
	UFUNCTION()
		void SetDotColor(FLinearColor NewValue) { ReticleProfiles[CurrentReticleIdx].DotColor = NewValue; }

	UFUNCTION()
		FLinearColor GetInlineColor() const { return ReticleProfiles[CurrentReticleIdx].InlineColor; }
	UFUNCTION()
		void SetInlineColor(FLinearColor NewValue) { ReticleProfiles[CurrentReticleIdx].InlineColor = NewValue; }

	UFUNCTION()
		FLinearColor GetOutlineColor() const { return ReticleProfiles[CurrentReticleIdx].OutlineColor; }
	UFUNCTION()
		void SetOutlineColor(FLinearColor NewValue) { ReticleProfiles[CurrentReticleIdx].OutlineColor = NewValue; }

	UFUNCTION()
		bool GetShowCenterDot() const { return ReticleProfiles[CurrentReticleIdx].bShowCenterDot; }
	UFUNCTION()
		void SetShowCenterDot(bool NewValue) { ReticleProfiles[CurrentReticleIdx].bShowCenterDot = NewValue; }

	UFUNCTION()
		double GetDotSize() const { return ReticleProfiles[CurrentReticleIdx].DotSize; }
	UFUNCTION()
		void SetDotSize(double NewValue) { ReticleProfiles[CurrentReticleIdx].DotSize = NewValue; }

	UFUNCTION()
		double GetOutlineThickness() const { return ReticleProfiles[CurrentReticleIdx].OutlineThickness; }
	UFUNCTION()
		void SetOutlineThickness(double NewValue) { ReticleProfiles[CurrentReticleIdx].OutlineThickness = NewValue; }

	UFUNCTION()
		double GetLineThicknessHori() const { return ReticleProfiles[CurrentReticleIdx].LineThicknessHori; }
	UFUNCTION()
		void SetLineThicknessHori(double NewValue) { ReticleProfiles[CurrentReticleIdx].LineThicknessHori = NewValue; }

	UFUNCTION()
		double GetLineLengthHori() const { return ReticleProfiles[CurrentReticleIdx].LineLengthHori; }
	UFUNCTION()
		void SetLineLengthHori(double NewValue) { ReticleProfiles[CurrentReticleIdx].LineLengthHori = NewValue; }

	UFUNCTION()
		double GetLineOffsetHori() const { return ReticleProfiles[CurrentReticleIdx].LineOffsetHori; }
	UFUNCTION()
		void SetLineOffsetHori(double NewValue) { ReticleProfiles[CurrentReticleIdx].LineOffsetHori = NewValue; }

	UFUNCTION()
		double GetLineThicknessVert() const { return ReticleProfiles[CurrentReticleIdx].LineThicknessVert; }
	UFUNCTION()
		void SetLineThicknessVert(double NewValue) { ReticleProfiles[CurrentReticleIdx].LineThicknessVert = NewValue; }

	UFUNCTION()
		double GetLineLengthVert() const { return ReticleProfiles[CurrentReticleIdx].LineLengthVert; }
	UFUNCTION()
		void SetLineLengthVert(double NewValue) { ReticleProfiles[CurrentReticleIdx].LineLengthVert = NewValue; }

	UFUNCTION()
		double GetLineOffsetVert() const { return ReticleProfiles[CurrentReticleIdx].LineOffsetVert; }
	UFUNCTION()
		void SetLineOffsetVert(double NewValue) { ReticleProfiles[CurrentReticleIdx].LineOffsetVert = NewValue; }

	UFUNCTION()
		bool GetShowHitMarker() const { return bShowHitMarker; }
	UFUNCTION()
		void SetShowHitMarker(bool NewValue) { bShowHitMarker = NewValue; }

	UFUNCTION()
		bool GetShowEliminationMarker() const { return bShowEliminationMarker; }
	UFUNCTION()
		void SetShowEliminationMarker(bool NewValue) { bShowEliminationMarker = NewValue; }

	UFUNCTION()
		bool GetApplyFiringError() const { return bApplyFiringError; }
	UFUNCTION()
		void SetApplyFiringError(bool NewValue) { bApplyFiringError = NewValue; }

private:
	UPROPERTY()
		bool bOverrideTargetingReticleColor = false;

	UPROPERTY()
		FLinearColor TargetingReticleColor = FLinearColor::Red;

	UPROPERTY()
		EReticleStyle ReticleStyle = EReticleStyle::Default;

	UPROPERTY()
		TArray<struct FReticle> ReticleProfiles;

	UPROPERTY()
		int32 CurrentReticleIdx;

	UPROPERTY()
		bool bShowHitMarker = true;

	UPROPERTY()
		bool bShowEliminationMarker = true;

	UPROPERTY()
		bool bApplyFiringError = true;

	// ゲームプレイ設定
	////////////////////////////////////////


	////////////////////////////////////////
	// コントロール設定

	// ===== マウス感度 =====
public:
	UFUNCTION()
		double GetMouseSensitivityX() const { return MouseSensitivityX; }
	UFUNCTION()
		void SetMouseSensitivityX(double NewValue) { ChangeValueAndDirty(MouseSensitivityX, NewValue); ApplyInputSensitivity(); }

	UFUNCTION()
		double GetMouseSensitivityY() const { return MouseSensitivityY; }
	UFUNCTION()
		void SetMouseSensitivityY(double NewValue) { ChangeValueAndDirty(MouseSensitivityY, NewValue); ApplyInputSensitivity(); }

	UFUNCTION()
		double GetTargetingMultiplier() const { return TargetingMultiplier; }
	UFUNCTION()
		void SetTargetingMultiplier(double NewValue) { ChangeValueAndDirty(TargetingMultiplier, NewValue); ApplyInputSensitivity(); }

	UFUNCTION()
		bool GetInvertVerticalAxis() const { return bInvertVerticalAxis; }
	UFUNCTION()
		void SetInvertVerticalAxis(bool NewValue) { ChangeValueAndDirty(bInvertVerticalAxis, NewValue); ApplyInputSensitivity(); }

	UFUNCTION()
		bool GetInvertHorizontalAxis() const { return bInvertHorizontalAxis; }
	UFUNCTION()
		void SetInvertHorizontalAxis(bool NewValue) { ChangeValueAndDirty(bInvertHorizontalAxis, NewValue); ApplyInputSensitivity(); }

	void ApplyInputSensitivity();

private:
	/** Holds the mouse horizontal sensitivity */
	UPROPERTY()
		double MouseSensitivityX = 1.0;

	/** Holds the mouse vertical sensitivity */
	UPROPERTY()
		double MouseSensitivityY = 1.0;

	/** Multiplier applied while Targeting down sights. */
	UPROPERTY()
		double TargetingMultiplier = 0.5;

	/** If true then the vertical look axis should be inverted */
	UPROPERTY()
		bool bInvertVerticalAxis = false;

	/** If true then the horizontal look axis should be inverted */
	UPROPERTY()
		bool bInvertHorizontalAxis = false;


	// ===== コントローラー感度設定 =====
public:
	UFUNCTION()
		EBEGamepadSensitivity GetGamepadLookSensitivityPreset() const { return GamepadLookSensitivityPreset; }
	UFUNCTION()
		void SetLookSensitivityPreset(EBEGamepadSensitivity NewValue) { ChangeValueAndDirty(GamepadLookSensitivityPreset, NewValue); ApplyInputSensitivity(); }

	UFUNCTION()
		EBEGamepadSensitivity GetGamepadTargetingSensitivityPreset() const { return GamepadTargetingSensitivityPreset; }
	UFUNCTION()
		void SetGamepadTargetingSensitivityPreset(EBEGamepadSensitivity NewValue) { ChangeValueAndDirty(GamepadTargetingSensitivityPreset, NewValue); ApplyInputSensitivity(); }

private:
	UPROPERTY()
		EBEGamepadSensitivity GamepadLookSensitivityPreset = EBEGamepadSensitivity::Normal;
	UPROPERTY()
		EBEGamepadSensitivity GamepadTargetingSensitivityPreset = EBEGamepadSensitivity::Normal;


	// ===== トグル・ホールド設定 =====
public:
	UFUNCTION()
		bool GetDefaultRun() const { return bDefaultRun; }
	UFUNCTION()
		void SetDefaultRun(bool NewValue) { bDefaultRun = NewValue; }

	UFUNCTION()
		bool GetUseToggleADS() const { return bUseToggleADS; }
	UFUNCTION()
		void SetUseToggleADS(bool NewValue) { bUseToggleADS = NewValue; }

	UFUNCTION()
		bool GetUseToggleRun() const { return bUseToggleRun; }
	UFUNCTION()
		void SetUseToggleRun(bool NewValue) { bUseToggleRun = NewValue; }

	UFUNCTION()
		bool GetUseToggleCrouch() const { return bUseToggleCrouch; }
	UFUNCTION()
		void SetUseToggleCrouch(bool NewValue) { bUseToggleCrouch = NewValue; }

	UFUNCTION()
		bool GetUseToggleOverlay() const { return bUseToggleOverlay; }
	UFUNCTION()
		void SetUseToggleOverlay(bool NewValue) { bUseToggleOverlay = NewValue; }

private:
	UPROPERTY()
		bool bDefaultRun;

	UPROPERTY()
		bool bUseToggleADS;

	UPROPERTY()
		bool bUseToggleRun;

	UPROPERTY()
		bool bUseToggleCrouch;

	UPROPERTY()
		bool bUseToggleOverlay;


	// ===== フォースフィードバック振動設定 =====
public:
	UFUNCTION()
		bool GetForceFeedbackEnabled() const { return bForceFeedbackEnabled; }

	UFUNCTION()
		void SetForceFeedbackEnabled(const bool NewValue) { ChangeValueAndDirty(bForceFeedbackEnabled, NewValue); }

private:
	/** Is force feedback enabled when a controller is being used? */
	UPROPERTY()
		bool bForceFeedbackEnabled = true;


	// ===== ハプティックフィードバック設定 =====
public:
	UFUNCTION()
		bool GetTriggerHapticsEnabled() const { return bTriggerHapticsEnabled; }
	UFUNCTION()
		void SetTriggerHapticsEnabled(const bool NewValue) { ChangeValueAndDirty(bTriggerHapticsEnabled, NewValue); }

	UFUNCTION()
		bool GetTriggerPullUsesHapticThreshold() const { return bTriggerPullUsesHapticThreshold; }
	UFUNCTION()
		void SetTriggerPullUsesHapticThreshold(const bool NewValue) { ChangeValueAndDirty(bTriggerPullUsesHapticThreshold, NewValue); }

	UFUNCTION()
		uint8 GetTriggerHapticStrength() const { return TriggerHapticStrength; }
	UFUNCTION()
		void SetTriggerHapticStrength(const uint8 NewValue) { ChangeValueAndDirty(TriggerHapticStrength, NewValue); }

	UFUNCTION()
		uint8 GetTriggerHapticStartPosition() const { return TriggerHapticStartPosition; }
	UFUNCTION()
		void SetTriggerHapticStartPosition(const uint8 NewValue) { ChangeValueAndDirty(TriggerHapticStartPosition, NewValue); }

private:
	/** Are trigger haptics enabled? */
	UPROPERTY()
		bool bTriggerHapticsEnabled = false;
	/** Does the game use the haptic feedback as its threshold for judging button presses? */
	UPROPERTY()
		bool bTriggerPullUsesHapticThreshold = true;
	/** The strength of the trigger haptic effects. */
	UPROPERTY()
		uint8 TriggerHapticStrength = 8;
	/** The start position of the trigger haptic effects */
	UPROPERTY()
		uint8 TriggerHapticStartPosition = 0;


	// ===== コントローラーのデッドゾーン設定 =====
public:
	/** Getter for gamepad move stick dead zone value. */
	UFUNCTION()
		float GetGamepadMoveStickDeadZone() const { return GamepadMoveStickDeadZone; }

	/** Setter for gamepad move stick dead zone value. */
	UFUNCTION()
		void SetGamepadMoveStickDeadZone(const float NewValue) { ChangeValueAndDirty(GamepadMoveStickDeadZone, NewValue); }

	/** Getter for gamepad look stick dead zone value. */
	UFUNCTION()
		float GetGamepadLookStickDeadZone() const { return GamepadLookStickDeadZone; }

	/** Setter for gamepad look stick dead zone value. */
	UFUNCTION()
		void SetGamepadLookStickDeadZone(const float NewValue) { ChangeValueAndDirty(GamepadLookStickDeadZone, NewValue); }

private:
	/** Holds the gamepad move stick dead zone value. */
	UPROPERTY()
		float GamepadMoveStickDeadZone;

	/** Holds the gamepad look stick dead zone value. */
	UPROPERTY()
		float GamepadLookStickDeadZone;

	// コントロール設定
	////////////////////////////////////////


	////////////////////////////////////////
	// ビデオ設定

	// ===== 色覚特性設定 =====
public:
	UFUNCTION()
		EColorBlindMode GetColorBlindMode() const;
	UFUNCTION()
		void SetColorBlindMode(EColorBlindMode InMode);

	UFUNCTION()
		int32 GetColorBlindStrength() const;
	UFUNCTION()
		void SetColorBlindStrength(int32 InColorBlindStrength);

private:
	UPROPERTY()
		EColorBlindMode ColorBlindMode = EColorBlindMode::Off;

	UPROPERTY()
		int32 ColorBlindStrength = 10;

	// ビデオ設定
	////////////////////////////////////////


	////////////////////////////////////////
	// オーディオ設定

	// ===== 画面外での音楽の再生設定 =====
public:
	UFUNCTION()
		EBEAllowBackgroundAudioSetting GetAllowAudioInBackgroundSetting() const { return AllowAudioInBackground; }
	UFUNCTION()
		void SetAllowAudioInBackgroundSetting(EBEAllowBackgroundAudioSetting NewValue);

	void ApplyAudioSettings();
private:
	UPROPERTY()
		EBEAllowBackgroundAudioSetting AllowAudioInBackground = EBEAllowBackgroundAudioSetting::Off;

	// オーディオ設定
	////////////////////////////////////////


	//======================================
	//	操作
	//======================================
public:
	/**
	 *  設定を保存する
	 */
	void SaveSettings();

	/**
	 *  設定を適応する
	 */
	void ApplySettings();

	/**
	 *  変更済みフラグを削除
	 */
	void ClearDirtyFlag() { bIsDirty = false; }

private:
	template<typename T>
	bool ChangeValueAndDirty(T& CurrentValue, const T& NewValue)
	{
		if (CurrentValue != NewValue)
		{
			CurrentValue = NewValue;
			bIsDirty = true;
			OnSettingChanged.Broadcast(this);

			return true;
		}

		return false;
	}

	bool bIsDirty = false;


	//======================================
	//	ユーティリティ
	//======================================
public:
	/**
	 *  このクラスのインスタンスが存在する場合にそれ取得する
	 *
	 * @Param LocalPlayer 設定を保有するプレイヤー
	 * @Return 設定のインスタンス
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameSettings", meta = (DisplayName = "GetBESharedSettings"))
	static UBESettingsShared* Get(const ULocalPlayer* LocalPlayer);

	static UBESettingsShared* LoadOrCreateSettings(const ULocalPlayer* LocalPlayer);

	/**
	 *  設定が編集済みかどうかを取得
	 *
	 * @Return 設定のインスタンス
	 */
	bool IsDirty() const { return bIsDirty; }

	//======================================
	//	イベント
	//======================================
public:
	DECLARE_EVENT_OneParam(UBESettingsShared, FOnSettingChangedEvent, UBESettingsShared* Settings);
	FOnSettingChangedEvent OnSettingChanged;

	FOnSettingChangedEvent OnSettingApplied;
};