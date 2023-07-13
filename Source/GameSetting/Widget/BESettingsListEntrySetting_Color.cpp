// Copyright owoDra

#include "BESettingsListEntrySetting_Color.h"

#include "GameSetting/SettingValue/BESettingValueColor.h"

#include "Input/Events.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Optional.h"
#include "Templates/Casts.h"
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTemplate.h"
#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingsListEntrySetting_Color)

//////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BESettings"

void UBESettingListEntrySetting_Color::SetSetting(UGameSetting* InSetting)
{
	ColorSetting = Cast<UBESettingValueColor>(InSetting);

	Super::SetSetting(InSetting);

	Refresh();
}

void UBESettingListEntrySetting_Color::Refresh()
{
	if (ensure(ColorSetting))
	{
		const FText Text = ColorSetting->GetFormattedText();

		Edit_SettingValue->SetText(Text);

		TOptional<FLinearColor> DefaultValue = ColorSetting->GetDefaultValue();
		OnDefaultValueChanged(DefaultValue.IsSet() ? DefaultValue.GetValue() : FLinearColor());

		const FLinearColor Value = ColorSetting->GetValue();
		OnValueChanged(Value);
	}
}

void UBESettingListEntrySetting_Color::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Edit_SettingValue->OnTextCommitted.AddDynamic(this, &ThisClass::HandleTextCommit);
}

void UBESettingListEntrySetting_Color::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	ColorSetting = nullptr;
}

void UBESettingListEntrySetting_Color::OnSettingChanged()
{
	Refresh();
}

void UBESettingListEntrySetting_Color::HandleTextCommit(const FText& Text, ETextCommit::Type CommitMethod)
{
	TGuardValue<bool> Suspend(bSuspendChangeUpdates, true);

	ColorSetting->SetValueFromHEXString(Text.ToString());

	Edit_SettingValue->SetText(ColorSetting->GetFormattedText());

	OnValueChanged(ColorSetting->GetValue());
}

void UBESettingListEntrySetting_Color::ChangeValue(FLinearColor Value)
{
	TGuardValue<bool> Suspend(bSuspendChangeUpdates, true);

	ColorSetting->SetValue(Value);

	Edit_SettingValue->SetText(ColorSetting->GetFormattedText());

	OnValueChanged(Value);
}

void UBESettingListEntrySetting_Color::HandleEditConditionChanged(UGameSetting* InSetting)
{
	Refresh();
}

void UBESettingListEntrySetting_Color::RefreshEditableState(const FGameSettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Edit_SettingValue->SetIsEnabled(bLocalIsEnabled);
	Panel_Value->SetIsEnabled(bLocalIsEnabled);
}

#undef LOCTEXT_NAMESPACE
