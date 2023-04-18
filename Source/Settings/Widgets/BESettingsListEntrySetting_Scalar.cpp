// Copyright Eigi Chin

#include "BESettingsListEntrySetting_Scalar.h"

#include "CommonButtonBase.h"
#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"
#include "GameSettingValueScalar.h"
#include "Input/Events.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Optional.h"
#include "Templates/Casts.h"
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTemplate.h"
#include "AnalogSlider.h"
#include "Components/SpinBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingsListEntrySetting_Scalar)

//////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BESettings"

void UBESettingListEntrySetting_Scalar::SetSetting(UGameSetting* InSetting)
{
	ScalarSetting = Cast<UGameSettingValueScalar>(InSetting);

	Super::SetSetting(InSetting);

	Refresh();
}

void UBESettingListEntrySetting_Scalar::Refresh()
{
	if (ensure(ScalarSetting))
	{
		const float Value = ScalarSetting->GetValueNormalized();

		Slider_SettingValue->SetValue(Value);
		Slider_SettingValue->SetStepSize(ScalarSetting->GetNormalizedStepSize());

		const TRange<double> Range = ScalarSetting->GetSourceRange();
		Spin_SettingValue->SetMaxValue(Range.GetUpperBoundValue());
		Spin_SettingValue->SetMaxSliderValue(Range.GetUpperBoundValue());
		Spin_SettingValue->SetMinValue(Range.GetLowerBoundValue());
		Spin_SettingValue->SetMinSliderValue(Range.GetLowerBoundValue());
		Spin_SettingValue->SetValue(ScalarSetting->GetValue());
		Spin_SettingValue->SetDelta(ScalarSetting->GetSourceStep());

		TOptional<double> DefaultValue = ScalarSetting->GetDefaultValueNormalized();
		OnDefaultValueChanged(DefaultValue.IsSet() ? DefaultValue.GetValue() : -1.0);

		OnValueChanged(Value);
	}
}

void UBESettingListEntrySetting_Scalar::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Slider_SettingValue->OnValueChanged.AddDynamic(this, &ThisClass::HandleSliderValueChanged);
	Slider_SettingValue->OnMouseCaptureEnd.AddDynamic(this, &ThisClass::HandleSliderValueCommited);
	Slider_SettingValue->OnControllerCaptureEnd.AddDynamic(this, &ThisClass::HandleSliderValueCommited);

	Spin_SettingValue->OnValueCommitted.AddDynamic(this, &ThisClass::HandleSpinValueCommited);
}

void UBESettingListEntrySetting_Scalar::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	ScalarSetting = nullptr;
}

void UBESettingListEntrySetting_Scalar::OnSettingChanged()
{
	Refresh();
}

void UBESettingListEntrySetting_Scalar::HandleSliderValueChanged(float Value)
{
	ChangeValue(Value);
}

void UBESettingListEntrySetting_Scalar::HandleSliderValueCommited()
{
	TGuardValue<bool> Suspend(bSuspendChangeUpdates, true);
}

void UBESettingListEntrySetting_Scalar::HandleSpinValueCommited(float Value, ETextCommit::Type CommitMethod)
{
	double NewValue = Value;
	double NomalizedValue = FMath::GetMappedRangeValueClamped(ScalarSetting->GetSourceRange(), TRange<double>(0, 1), NewValue);
	ChangeValue(NomalizedValue);
}

void UBESettingListEntrySetting_Scalar::ChangeValue(float Value)
{
	TGuardValue<bool> Suspend(bSuspendChangeUpdates, true);

	ScalarSetting->SetValueNormalized(Value);

	Value = ScalarSetting->GetValueNormalized();

	Slider_SettingValue->SetValue(Value);
	Spin_SettingValue->SetValue(ScalarSetting->GetValue());

	OnValueChanged(Value);
}

void UBESettingListEntrySetting_Scalar::RefreshEditableState(const FGameSettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Slider_SettingValue->SetIsEnabled(bLocalIsEnabled);
	Spin_SettingValue->SetIsEnabled(bLocalIsEnabled);
	Panel_Value->SetIsEnabled(bLocalIsEnabled);
}

#undef LOCTEXT_NAMESPACE
