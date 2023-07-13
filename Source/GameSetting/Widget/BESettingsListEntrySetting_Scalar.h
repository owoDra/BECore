// Copyright owoDra

#pragma once

#include "Widgets/GameSettingListEntry.h"

#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"
#include "GameSettingFilterState.h"
#include "HAL/Platform.h"
#include "Input/Reply.h"
#include "Internationalization/Text.h"
#include "Containers/UnrealString.h"
#include "UObject/UObjectGlobals.h"

#include "BESettingsListEntrySetting_Scalar.generated.h"

class UAnalogSlider;
class USpinBox;
class UGameSetting;
class UGameSettingValueScalar;
class UObject;
class UPanelWidget;
class UUserWidget;
class UWidget;
struct FFocusEvent;
struct FFrame;
struct FGeometry;

//////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class UBESettingListEntrySetting_Scalar : public UGameSettingListEntry_Setting
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
		TObjectPtr<UGameSettingValueScalar> ScalarSetting;


	//====================================
	// イベント
	//====================================
protected:
	// スライダーウィジェットからバインド
	UFUNCTION()
		void HandleSliderValueChanged(float Value);
	UFUNCTION()
		void HandleSliderValueCommited();

	// スピナーウィジェットからバインド
	UFUNCTION()
		void HandleSpinValueCommited(float Value, ETextCommit::Type CommitMethod);

	void ChangeValue(float Value);

	UFUNCTION(BlueprintImplementableEvent)
		void OnValueChanged(float Value);

	UFUNCTION(BlueprintImplementableEvent)
		void OnDefaultValueChanged(float DefaultValue);

	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;


	//====================================
	// バインド
	//====================================
private:	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UPanelWidget> Panel_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<UAnalogSlider> Slider_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
		TObjectPtr<USpinBox> Spin_SettingValue;
};
