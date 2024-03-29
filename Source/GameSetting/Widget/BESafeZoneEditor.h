// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CommonActivatableWidget.h"

#include "CommonInputBaseTypes.h"
#include "Delegates/Delegate.h"
#include "Input/Reply.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Widgets/IGameSettingActionInterface.h"

#include "BESafeZoneEditor.generated.h"

class UCommonButtonBase;
class UCommonRichTextBlock;
class UGameSetting;
class UGameSettingValueScalar;
class UObject;
class UWidgetSwitcher;
struct FAnalogInputEvent;
struct FFrame;
struct FGameplayTag;
struct FGeometry;
struct FPointerEvent;

//////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class UBESafeZoneEditor : public UCommonActivatableWidget, public IGameSettingActionInterface
{
	GENERATED_BODY()

public:
	FSimpleMulticastDelegate OnSafeZoneSet;
	
public:
	UBESafeZoneEditor(const FObjectInitializer& Initializer);

	// Begin IGameSettingActionInterface
	virtual bool ExecuteActionForSetting_Implementation(FGameplayTag ActionTag, UGameSetting* InSetting) override;
	// End IGameSettingActionInterface

protected:

	UPROPERTY(EditAnywhere, Category = "Restrictions")
	bool bCanCancel = true;

	virtual void NativeOnActivated() override;
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	void HandleInputModeChanged(ECommonInputType InInputType);

private:
	UFUNCTION()
	void HandleBackClicked();

	UFUNCTION()
	void HandleDoneClicked();

	TWeakObjectPtr<UGameSettingValueScalar> ValueSetting;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
	UWidgetSwitcher* Switcher_SafeZoneMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
	UCommonRichTextBlock* RichText_Default;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
	UCommonButtonBase* Button_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = true))
	UCommonButtonBase* Button_Done;
};
