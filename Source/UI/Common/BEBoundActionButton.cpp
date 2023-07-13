// Copyright owoDra

#include "BEBoundActionButton.h"
#include "CommonInputSubsystem.h"

void UBEBoundActionButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
		HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
	}
}

void UBEBoundActionButton::HandleInputMethodChanged(ECommonInputType NewInputMethod)
{
	TSubclassOf<UCommonButtonStyle> NewStyle = nullptr;

	if (NewInputMethod == ECommonInputType::Gamepad)
	{
		NewStyle = GamepadStyle;
	}
	else if (NewInputMethod == ECommonInputType::Touch)
	{
		NewStyle = TouchStyle;
	}
	else
	{
		NewStyle = KeyboardStyle;
	}

	if (NewStyle)
	{
		SetStyle(NewStyle);
	}
}
