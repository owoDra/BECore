// Copyright Eigi Chin

#include "BESettingValueColorDynamic.h"

#include "DataSource/GameSettingDataSourceDynamic.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingValueColorDynamic)

//////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BESettings"

UBESettingValueColorDynamic::UBESettingValueColorDynamic()
{
}

void UBESettingValueColorDynamic::Startup()
{
	Getter->Startup(LocalPlayer, FSimpleDelegate::CreateUObject(this, &ThisClass::OnDataSourcesReady));
}

void UBESettingValueColorDynamic::StoreInitial()
{
	InitialValue = GetValue();
}

void UBESettingValueColorDynamic::ResetToDefault()
{
	if (DefaultValue.IsSet())
	{
		SetValue(DefaultValue.GetValue(), EGameSettingChangeReason::ResetToDefault);
	}
}

void UBESettingValueColorDynamic::RestoreToInitial()
{
	SetValue(InitialValue, EGameSettingChangeReason::RestoreToInitial);
}

void UBESettingValueColorDynamic::SetDynamicGetter(const TSharedRef<FGameSettingDataSource>& InGetter)
{
	Getter = InGetter;
}

void UBESettingValueColorDynamic::SetDynamicSetter(const TSharedRef<FGameSettingDataSource>& InSetter)
{
	Setter = InSetter;
}

void UBESettingValueColorDynamic::SetDefaultValue(FLinearColor InValue)
{
	DefaultValue = InValue;
}

void UBESettingValueColorDynamic::OnInitialized()
{
#if !UE_BUILD_SHIPPING
	ensureAlways(Getter);
	ensureAlwaysMsgf(Getter->Resolve(LocalPlayer), TEXT("%s: %s did not resolve, are all functions and properties valid, and are they UFunctions/UProperties?"), *GetDevName().ToString(), *Getter->ToString());
	ensureAlways(Setter);
	ensureAlwaysMsgf(Setter->Resolve(LocalPlayer), TEXT("%s: %s did not resolve, are all functions and properties valid, and are they UFunctions/UProperties?"), *GetDevName().ToString(), *Setter->ToString());
#endif

	Super::OnInitialized();
}

void UBESettingValueColorDynamic::OnDataSourcesReady()
{
	StartupComplete();
}

TOptional<FLinearColor> UBESettingValueColorDynamic::GetDefaultValue() const
{
	return DefaultValue;
}

void UBESettingValueColorDynamic::SetValue(FLinearColor Value, EGameSettingChangeReason Reason)
{
	const FString StringValue = Value.ToString();
	Setter->SetValue(LocalPlayer, StringValue);

	NotifySettingChanged(Reason);
}

FLinearColor UBESettingValueColorDynamic::GetValue() const
{
	const FString OutValue = Getter->GetValueAsString(LocalPlayer);

	FLinearColor Value;
	Value.InitFromString(OutValue);

	return Value;
}

#undef LOCTEXT_NAMESPACE
