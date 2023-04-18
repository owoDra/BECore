// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Widgets/GameSettingListEntry.h"

#include "HAL/Platform.h"
#include "InputCoreTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Misc/KeyAlreadyBoundWarning.h"

#include "BESettingsListEntrySetting_KeyConfig.generated.h"

class UBESettingValueKeyConfig;
class UBEButtonBase;
class UGameSetting;
class UGameSettingPressAnyKey;
class UObject;

//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class UBESettingsListEntrySetting_KeyConfig : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

	//====================================
	// 初期化
	//====================================
public:
	virtual void SetSetting(UGameSetting* InSetting) override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void OnSettingChanged() override;

	void Refresh();

	UFUNCTION(BlueprintImplementableEvent, Category = "GameSettings|KeyConfig", meta = (DisplayName = "OnRefresh"))
	void K2_Refresh(const TArray<FKey>& BoundKeys);

private:
	UPROPERTY(Transient)
		FKey OriginalKeyToBind = EKeys::Invalid;

	UPROPERTY(Transient)
		int32 TargetSlotToBind;

protected:
	UPROPERTY()
		TObjectPtr<UBESettingValueKeyConfig> KeyConfigSetting;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UGameSettingPressAnyKey> PressAnyKeyPanelClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UKeyAlreadyBoundWarning> KeyAlreadyBoundWarningPanelClass;

	//====================================
	// 設定
	//====================================
public:
	UPROPERTY(EditDefaultsOnly)
		int MaxMappableKeys = 5;


	//====================================
	// 操作
	//====================================
protected:
	UFUNCTION(BlueprintCallable, Category = "GameSettings|KeyConfig")
	void ChangeBinding(int32 BindSlot, FKey InKey);


	//====================================
	// ユーティリティ
	//====================================
protected:
	/**
	 *  現在空いているスロットを取得
	 * 
	 * @Param BoundKeys バインドされたキー配列
	 * @Return 見つかったスロット。無い場合は-1。
	 */
	int32 FindEmptySlot(TArray<FKey>& BoundKeys);

	
	//====================================
	// イベント
	//====================================
protected:
	void HandleAddKeyClicked();

	void HandleKeySelected(FKey InKey, UGameSettingPressAnyKey* PressAnyKeyPanel);
	void HandleDuplicateKeySelected(FKey InKey, UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const;

	void HandleKeySelectionCanceled(UGameSettingPressAnyKey* PressAnyKeyPanel);
	void HandleKeySelectionCanceled(UKeyAlreadyBoundWarning* AlreadyBoundWarningPanel);
	

	//====================================
	// バインド
	//====================================
private:	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UBEButtonBase> Button_AddKey;
};
