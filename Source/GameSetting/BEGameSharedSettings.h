// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GameFramework/SaveGame.h"

#include "CoreMinimal.h"
#include "SubtitleDisplaySubsystem.h"

#include "BEGameSharedSettings.generated.h"

class ULocalPlayer;
class UBELocalPlayer;


/**
 * EColorBlindMode
 * 
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
 * EBEGamepadSensitivity
 * 
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


/**
 * UBEGameSharedSettings 
 * 
 *  このゲームの設定を保存するためのベースクラス。
 *  このクラスを継承することで設定の項目を拡張することができる。
 *  保存したデータは SvaeGame として保存される。
 */
UCLASS()
class BECORE_API UBEGameSharedSettings : public USaveGame
{
	GENERATED_BODY()
public:
	UBEGameSharedSettings() {}

	// Shared設定の保存するスロット名
	static const FString NAME_SharedSettings_SaveSlot;

protected:
	/**
	 * Initialize
	 *
	 *  この設定の初期化を行う
	 */
	void Initialize(ULocalPlayer* LocalPlayer);
	virtual void OnInitialized(ULocalPlayer* LocalPlayer) {}

private:
	UPROPERTY(Transient)
	UBELocalPlayer* OwningPlayer = nullptr;


public:
	/**
	 * SaveSettings
	 *
	 *  設定を保存する
	 */
	void SaveSettings();

	/**
	 * ApplySettings
	 *
	 *  設定を適応する
	 */
	void ApplySettings();

	/**
	 * ClearDirtyFlag
	 *
	 *  変更済みフラグを削除
	 */
	void ClearDirtyFlag() { bIsDirty = false; }

	/**
	 * IsDirty
	 * 
	 *  設定が編集済みかどうかを取得
	 */
	bool IsDirty() const { return bIsDirty; }

protected:
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

private:
	// 設定が変更されたか判定する
	bool bIsDirty = false;


public:
	/**
	 * LoadOrCreateSettings
	 *
	 *  設定を読み込みまたは作成する
	 */
	static UBEGameSharedSettings* LoadOrCreateSettings(const ULocalPlayer* LocalPlayer);


	//======================================
	//	イベント
	//======================================
public:
	DECLARE_EVENT_OneParam(UBEGameSharedSettings, FOnSettingChangedEvent, UBEGameSharedSettings* Settings);
	FOnSettingChangedEvent OnSettingChanged;

	FOnSettingChangedEvent OnSettingApplied;


	//======================================
	//	設定
	//======================================

	////////////////////////////////////////
	// コントロール設定

	// ===== 共有系 | 反転設定 =====
private:
	// 縦軸の反転
	UPROPERTY() bool bInvertVerticalAxis = false;

	// 横軸の反転
	UPROPERTY() bool bInvertHorizontalAxis = false;

public:
	/**
	 *  縦軸の反転
	 */
	UFUNCTION() bool GetInvertVerticalAxis() const { return bInvertVerticalAxis; }
	UFUNCTION() void SetInvertVerticalAxis(bool NewValue) { ChangeValueAndDirty(bInvertVerticalAxis, NewValue); }

	/**
	 *  横軸の反転
	 */
	UFUNCTION() bool GetInvertHorizontalAxis() const { return bInvertHorizontalAxis; }
	UFUNCTION() void SetInvertHorizontalAxis(bool NewValue) { ChangeValueAndDirty(bInvertHorizontalAxis, NewValue); }


	// ===== キーボードマウス系 =====
private:
	// 横軸へのマウス感度
	UPROPERTY() double MouseSensitivityX = 1.0;

	// 縦軸へのマウス感度
	UPROPERTY() double MouseSensitivityY = 1.0;

	// ターゲット時のマウス感度倍率 (銃のADSなどに使用)
	UPROPERTY() double TargetingMultiplier = 0.5;

public:
	/**
	 *  横軸へのマウス感度
	 */
	UFUNCTION() double GetMouseSensitivityX() const { return MouseSensitivityX; }
	UFUNCTION() void SetMouseSensitivityX(double NewValue) { ChangeValueAndDirty(MouseSensitivityX, NewValue); }

	/**
	 *  縦軸へのマウス感度
	 */
	UFUNCTION() double GetMouseSensitivityY() const { return MouseSensitivityY; }
	UFUNCTION() void SetMouseSensitivityY(double NewValue) { ChangeValueAndDirty(MouseSensitivityY, NewValue); }

	/**
	 *  ターゲット時のマウス感度倍率
	 */
	UFUNCTION() double GetTargetingMultiplier() const { return TargetingMultiplier; }
	UFUNCTION() void SetTargetingMultiplier(double NewValue) { ChangeValueAndDirty(TargetingMultiplier, NewValue); }


	// ===== ゲームコントローラー系 | 感度設定 =====
private:
	// コントローラーの感度設定
	UPROPERTY() EBEGamepadSensitivity GamepadLookSensitivityPreset = EBEGamepadSensitivity::Normal;

	// コントローラーのターゲット時の感度設定
	UPROPERTY() EBEGamepadSensitivity GamepadTargetingSensitivityPreset = EBEGamepadSensitivity::Normal;

public:
	/**
	 *  コントローラーの感度設定
	 */
	UFUNCTION() EBEGamepadSensitivity GetGamepadLookSensitivityPreset() const { return GamepadLookSensitivityPreset; }
	UFUNCTION() void SetLookSensitivityPreset(EBEGamepadSensitivity NewValue) { ChangeValueAndDirty(GamepadLookSensitivityPreset, NewValue); }

	/**
	 *  コントローラーのターゲット時の感度設定
	 */
	UFUNCTION() EBEGamepadSensitivity GetGamepadTargetingSensitivityPreset() const { return GamepadTargetingSensitivityPreset; }
	UFUNCTION() void SetGamepadTargetingSensitivityPreset(EBEGamepadSensitivity NewValue) { ChangeValueAndDirty(GamepadTargetingSensitivityPreset, NewValue); }


	// ===== ゲームコントローラー系 | デッドゾーン設定 =====
private:
	// 移動用のスティックのデッドゾーン
	UPROPERTY() float GamepadMoveStickDeadZone;

	// 視点操作用のスティックのデッドゾーン
	UPROPERTY() float GamepadLookStickDeadZone;

public:
	/**
	 *  移動用のスティックのデッドゾーン
	 */
	UFUNCTION() float GetGamepadMoveStickDeadZone() const { return GamepadMoveStickDeadZone; }
	UFUNCTION() void SetGamepadMoveStickDeadZone(const float NewValue) { ChangeValueAndDirty(GamepadMoveStickDeadZone, NewValue); }

	/**
	 *  視点操作用のスティックのデッドゾーン
	 */
	UFUNCTION() float GetGamepadLookStickDeadZone() const { return GamepadLookStickDeadZone; }
	UFUNCTION() void SetGamepadLookStickDeadZone(const float NewValue) { ChangeValueAndDirty(GamepadLookStickDeadZone, NewValue); }


	// ===== ゲームコントローラー系 | 振動設定 =====
private:
	// 振動を有効にするかどうか
	UPROPERTY() bool bForceFeedbackEnabled = true;

public:
	/**
	 *  振動設定
	 */
	UFUNCTION() bool GetForceFeedbackEnabled() const { return bForceFeedbackEnabled; }
	UFUNCTION() void SetForceFeedbackEnabled(const bool NewValue) { ChangeValueAndDirty(bForceFeedbackEnabled, NewValue); }


	// ===== ゲームコントローラー系 | 触覚フィードバック設定 =====
private:
	// 触覚フィードバックをするかどうか
	UPROPERTY() bool bTriggerHapticsEnabled = false;
	
	// トリガーを引いたときに任意の位置でフィードバックをするかどうか
	UPROPERTY() bool bTriggerPullUsesHapticThreshold = true;
	
	// 触覚フィードバックの強さ
	UPROPERTY() uint8 TriggerHapticStrength = 8;
	
	// 触覚フィードバックを行うトリガー位置
	UPROPERTY() uint8 TriggerHapticStartPosition = 0;

public:
	/**
	 *  触覚フィードバックの有効化設定
	 */
	UFUNCTION() bool GetTriggerHapticsEnabled() const { return bTriggerHapticsEnabled; }
	UFUNCTION() void SetTriggerHapticsEnabled(const bool NewValue) { ChangeValueAndDirty(bTriggerHapticsEnabled, NewValue); }

	/**
	 *  触覚フィードバックのトリガー位置の有効化設定
	 */
	UFUNCTION() bool GetTriggerPullUsesHapticThreshold() const { return bTriggerPullUsesHapticThreshold; }
	UFUNCTION() void SetTriggerPullUsesHapticThreshold(const bool NewValue) { ChangeValueAndDirty(bTriggerPullUsesHapticThreshold, NewValue); }

	/**
	 *  触覚フィードバックの強度設定
	 */
	UFUNCTION() uint8 GetTriggerHapticStrength() const { return TriggerHapticStrength; }
	UFUNCTION() void SetTriggerHapticStrength(const uint8 NewValue) { ChangeValueAndDirty(TriggerHapticStrength, NewValue); }

	/**
	 *  触覚フィードバックのトリガー位置設定
	 */
	UFUNCTION() uint8 GetTriggerHapticStartPosition() const { return TriggerHapticStartPosition; }
	UFUNCTION() void SetTriggerHapticStartPosition(const uint8 NewValue) { ChangeValueAndDirty(TriggerHapticStartPosition, NewValue); }


	////////////////////////////////////////
	// ゲームプレイ設定

	// ===== アクセシビリティ系 | 言語設定 =====
private:
	// 現在適応されている保留中の言語
	UPROPERTY(Transient) FString PendingCulture;

	// デフォルトの言語にリセットするかどうか。
	// True ならばリセットする。
	bool bResetToDefaultCulture = false;

public:
	/**
	 *  現在適応されている保留中の言語
	 */
	const FString& GetPendingCulture() const { return PendingCulture; }
	void SetPendingCulture(const FString& NewCulture);

	/**
	 * OnCultureChanged
	 * 
	 *  言語が変更されたときに実行される
	 */
	void OnCultureChanged();

	/**
	 * ClearPendingCulture
	 *
	 *  現在保留中の言語を削除する
	 */
	void ClearPendingCulture();

	/**
	 * IsUsingDefaultCulture
	 *
	 *  現在デフォルトの言語を使用しているかどうか
	 */
	bool IsUsingDefaultCulture() const;

	/**
	 * ResetToDefaultCulture
	 *
	 *  デフォルトの言語にリセットする
	 */
	void ResetToDefaultCulture();

	/**
	 * IsUsingDefaultCulture
	 *
	 *  現在デフォルトの言語を使用しているかどうか
	 */
	bool ShouldResetToDefaultCulture() const { return bResetToDefaultCulture; }

	/**
	 * ApplyCultureSettings
	 *
	 *  言語設定を適応する
	 */
	void ApplyCultureSettings();

	/**
	 * ResetCultureToCurrentSettings
	 *
	 *  適応中の言語を設定中の言語に戻す
	 */
	void ResetCultureToCurrentSettings();


	// ===== アクセシビリティ系 | 色覚特性設定 =====
private:
	// 色覚特性の種類
	UPROPERTY() EColorBlindMode ColorBlindMode = EColorBlindMode::Off;

	// 色覚特性の強さ
	UPROPERTY() int32 ColorBlindStrength = 10;

public:
	/**
	 *  色覚特性の種類
	 */
	UFUNCTION() EColorBlindMode GetColorBlindMode() const { return ColorBlindMode; }
	UFUNCTION() void SetColorBlindMode(EColorBlindMode InMode);

	/**
	 *  色覚特性の強さ
	 */
	UFUNCTION() int32 GetColorBlindStrength() const { return ColorBlindStrength; }
	UFUNCTION() void SetColorBlindStrength(int32 InColorBlindStrength);


	// ===== アクセシビリティ系 | 言語設定 =====
private:
	// 字幕関係の設定
	UPROPERTY() bool bEnableSubtitles = true;
	UPROPERTY() ESubtitleDisplayTextSize SubtitleTextSize = ESubtitleDisplayTextSize::Medium;
	UPROPERTY() ESubtitleDisplayTextColor SubtitleTextColor = ESubtitleDisplayTextColor::White;
	UPROPERTY() ESubtitleDisplayTextBorder SubtitleTextBorder = ESubtitleDisplayTextBorder::None;
	UPROPERTY() ESubtitleDisplayBackgroundOpacity SubtitleBackgroundOpacity = ESubtitleDisplayBackgroundOpacity::Medium;

public:
	/**
	 *  字幕の有効化
	 */
	UFUNCTION() bool GetSubtitlesEnabled() const { return bEnableSubtitles; }
	UFUNCTION() void SetSubtitlesEnabled(bool Value) { ChangeValueAndDirty(bEnableSubtitles, Value); }

	/**
	 *  字幕の大きさ
	 */
	UFUNCTION() ESubtitleDisplayTextSize GetSubtitlesTextSize() const { return SubtitleTextSize; }
	UFUNCTION() void SetSubtitlesTextSize(ESubtitleDisplayTextSize Value) { ChangeValueAndDirty(SubtitleTextSize, Value); ApplySubtitleOptions(); }

	/**
	 *  字幕の色
	 */
	UFUNCTION() ESubtitleDisplayTextColor GetSubtitlesTextColor() const { return SubtitleTextColor; }
	UFUNCTION() void SetSubtitlesTextColor(ESubtitleDisplayTextColor Value) { ChangeValueAndDirty(SubtitleTextColor, Value); ApplySubtitleOptions(); }

	/**
	 *  字幕の輪郭
	 */
	UFUNCTION() ESubtitleDisplayTextBorder GetSubtitlesTextBorder() const { return SubtitleTextBorder; }
	UFUNCTION() void SetSubtitlesTextBorder(ESubtitleDisplayTextBorder Value) { ChangeValueAndDirty(SubtitleTextBorder, Value); ApplySubtitleOptions(); }

	/**
	 *  字幕の透明度
	 */
	UFUNCTION() ESubtitleDisplayBackgroundOpacity GetSubtitlesBackgroundOpacity() const { return SubtitleBackgroundOpacity; }
	UFUNCTION() void SetSubtitlesBackgroundOpacity(ESubtitleDisplayBackgroundOpacity Value) { ChangeValueAndDirty(SubtitleBackgroundOpacity, Value); ApplySubtitleOptions(); }

	/**
	 * ApplySubtitleOptions
	 *
	 *  字幕系設定と適応する
	 */
	void ApplySubtitleOptions();
};