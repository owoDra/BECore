// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BESafeZoneEditor.h"

#include "GameSetting/BEGameDeviceSettings.h"

#include "CommonButtonBase.h"
#include "CommonRichTextBlock.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/WidgetSwitcher.h"
#include "Containers/Array.h"
#include "GameSetting.h"
#include "GameSettingValueScalar.h"
#include "GameplayTagContainer.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/Optional.h"
#include "Templates/Casts.h"
#include "UObject/WeakObjectPtr.h"
#include "Widgets/Layout/SSafeZone.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESafeZoneEditor)

struct FGeometry;

//////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BE"

namespace SafeZoneEditor
{
	const float JoystickDeadZone = 0.2f;
	const float SafeZoneChangeSpeed = 0.1f;	
}

//////////////////////////////////////////////////////////////////////

UBESafeZoneEditor::UBESafeZoneEditor(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
}

void UBESafeZoneEditor::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Switcher_SafeZoneMessage->SetActiveWidget(RichText_Default);
}

void UBESafeZoneEditor::NativeOnActivated()
{
	Super::NativeOnActivated();

	SSafeZone::SetGlobalSafeZoneScale(UBEGameDeviceSettings::Get()->GetSafeZone());
	
	Button_Done->OnClicked().AddUObject(this, &UBESafeZoneEditor::HandleDoneClicked);

	Button_Back->SetVisibility((bCanCancel)? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (bCanCancel)
	{
		Button_Back->OnClicked().AddUObject(this, &UBESafeZoneEditor::HandleBackClicked);
	}
}

bool UBESafeZoneEditor::ExecuteActionForSetting_Implementation(FGameplayTag ActionTag, UGameSetting* InSetting)
{
	TArray<UGameSetting*> ChildSettings = InSetting ? InSetting->GetChildSettings() : TArray<UGameSetting*>();
	if (ChildSettings.Num() > 0 && ChildSettings[0])
	{
		ValueSetting = Cast<UGameSettingValueScalar>(ChildSettings[0]);
	}

	return true;
}

FReply UBESafeZoneEditor::NativeOnAnalogValueChanged(const FGeometry& InGeometry, const FAnalogInputEvent& InAnalogEvent)
{
	if (InAnalogEvent.GetKey() == EKeys::Gamepad_LeftY && FMath::Abs(InAnalogEvent.GetAnalogValue()) >= SafeZoneEditor::JoystickDeadZone)
	{
		const float SafeZoneMultiplier = FMath::Clamp(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f) + InAnalogEvent.GetAnalogValue() * SafeZoneEditor::SafeZoneChangeSpeed, 0.0f, 1.0f);
		SSafeZone::SetGlobalSafeZoneScale(SafeZoneMultiplier >= 0 ? SafeZoneMultiplier : 0);
		
		return FReply::Handled();
	}
	return Super::NativeOnAnalogValueChanged(InGeometry, InAnalogEvent);
}

FReply UBESafeZoneEditor::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const float SafeZoneMultiplier = FMath::Clamp(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f) + InMouseEvent.GetWheelDelta() * SafeZoneEditor::SafeZoneChangeSpeed, 0.0f, 1.0f);
	SSafeZone::SetGlobalSafeZoneScale(SafeZoneMultiplier >= 0 ? SafeZoneMultiplier : 0);

	return FReply::Handled();
}

void UBESafeZoneEditor::HandleInputModeChanged(ECommonInputType InInputType)
{
	const FText KeyName = InInputType == ECommonInputType::Gamepad ? LOCTEXT("SafeZone_KeyToPress_Gamepad", "Left Stick") : LOCTEXT("SafeZone_KeyToPress_Mouse", "Mouse Wheel");
	RichText_Default->SetText(FText::Format(LOCTEXT("SafeZoneEditorInstructions", "Use <text color=\"FFFFFFFF\" fontface=\"black\">{0}</> to adjust the corners so it lines up with the edges of your display."), KeyName));
}

void UBESafeZoneEditor::HandleBackClicked()
{
	DeactivateWidget();
	SSafeZone::SetGlobalSafeZoneScale(UBEGameDeviceSettings::Get()->GetSafeZone());
}

void UBESafeZoneEditor::HandleDoneClicked()
{
	if (ValueSetting.IsValid())
	{
		ValueSetting.Get()->SetValue(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f));
	}
	else
	{
		UBEGameDeviceSettings::Get()->SetSafeZone(SSafeZone::GetGlobalSafeZoneScale().Get(1.0f));
	}
	OnSafeZoneSet.Broadcast();
	DeactivateWidget();
}

#undef LOCTEXT_NAMESPACE