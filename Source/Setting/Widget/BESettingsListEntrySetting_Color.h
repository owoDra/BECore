// Copyright Eigi Chin

#pragma once

#include "Widgets/GameSettingListEntry.h"

#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"
#include "GameSettingFilterState.h"
#include "HAL/Platform.h"
#include "Input/Reply.h"
#include "Internationalization/Text.h"
#include "Containers/UnrealString.h"
#include "Math/Color.h"
#include "UObject/UObjectGlobals.h"

#include "BESettingsListEntrySetting_Color.generated.h"

class UBESettingValueColor;
class UEditableTextBox;
class UPanelWidget;
class UObject;
struct FFocusEvent;
struct FFrame;
struct FGeometry;

//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class UBESettingListEntrySetting_Color : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

	//====================================
	// 初期化
	//====================================
public:
	virtual void SetSetting(UGameSetting* InSetting) override;

protected:
	void Refresh();
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void OnSettingChanged() override;

protected:
	UPROPERTY()
		TObjectPtr<UBESettingValueColor> ColorSetting;


	//====================================
	// イベント
	//====================================
protected:
	// エディタブルテキストウィジェットからバインド
	UFUNCTION()
		void HandleTextCommit(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION(BlueprintImplementableEvent)
		void OnValueChanged(FLinearColor Value);

	UFUNCTION(BlueprintImplementableEvent)
		void OnDefaultValueChanged(FLinearColor DefaultValue);

	UFUNCTION(BlueprintCallable, Category = "GameSettings")
		void ChangeValue(FLinearColor Value);

	virtual void HandleEditConditionChanged(UGameSetting* InSetting) override;
	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;


	//====================================
	// バインド
	//====================================
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UPanelWidget> Panel_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UEditableTextBox> Edit_SettingValue;
};
