// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BESettingValueKeyConfig.h"

#include "Settings/BESettingsLocal.h"
#include "Player/BELocalPlayer.h"

#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "GameSetting.h"
#include "GameSettingFilterState.h"
#include "Internationalization/Internationalization.h"
#include "Misc/AssertionMacros.h"
#include "PlayerMappableInputConfig.h"
#include "Templates/Casts.h"
#include "UObject/NameTypes.h"
#include "UObject/UnrealNames.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingValueKeyConfig)

class ULocalPlayer;

//////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BESettings"

void FKeyOption::ResetToDefault()
{
	if (OwningConfig)
	{
		InputMapping = OwningConfig->GetMappingByName(InputMapping.PlayerMappableOptions.Name);
	}
	// If we don't have an owning config, then there is no default binding for this and it can simply be removed
	else
	{
		InputMapping = FEnhancedActionKeyMapping();
	}
}

void FKeyOption::SetInitialValue(FKey InKey)
{
	InitialMapping = InKey;
}

//////////////////////////////////////////////////////////////////////

UBESettingValueKeyConfig::UBESettingValueKeyConfig()
{
	bReportAnalytics = false;
}

void UBESettingValueKeyConfig::OnInitialized()
{
	DynamicDetails = FGetGameSettingsDetails::CreateLambda([this](ULocalPlayer&) {
		return FText::Format(LOCTEXT("DynamicDetails_KeyboardInputAction", "Bindings for {0}"), MappableOptions[0].InputMapping.PlayerMappableOptions.DisplayName);
	});

	Super::OnInitialized();
}

void UBESettingValueKeyConfig::StoreInitial()
{
	for (int i = (MappableOptions.Num() - 1); i >= 0; i--)
	{
		MappableOptions[i].SetInitialValue(MappableOptions[i].InputMapping.Key);
	}
}

void UBESettingValueKeyConfig::SetInputData(FEnhancedActionKeyMapping& BaseMapping, const UPlayerMappableInputConfig* InOwningConfig, int32 InKeyBindSlot)
{
	FKeyOption NewKeyOption;
	NewKeyOption.InputMapping = BaseMapping;
	NewKeyOption.OwningConfig = InOwningConfig;
	NewKeyOption.SetInitialValue(BaseMapping.Key);

	MappableOptions.EmplaceAt(InKeyBindSlot, NewKeyOption);

	ensure(MappableOptions[0].InputMapping.PlayerMappableOptions.Name != NAME_None && !MappableOptions[0].InputMapping.PlayerMappableOptions.DisplayName.IsEmpty());

	const FString NameString = TEXT("KBM_Input_") + MappableOptions[0].InputMapping.PlayerMappableOptions.Name.ToString();
	SetDevName(*NameString);
	SetDisplayName(MappableOptions[0].InputMapping.PlayerMappableOptions.DisplayName);
}

void UBESettingValueKeyConfig::ResetToDefault()
{
	for (int i = (MappableOptions.Num() - 1); i >= 0; i--)
	{
		MappableOptions[i].ResetToDefault();
	}
}

void UBESettingValueKeyConfig::RestoreToInitial()
{
	for (int i = (MappableOptions.Num() - 1); i >= 0; i--)
	{
		ChangeBinding(i, MappableOptions[i].GetInitialStoredValue());
	}
}

bool UBESettingValueKeyConfig::ChangeBinding(int32 InKeyBindSlot, FKey NewKey)
{
	if (!MappableOptions.IsValidIndex(InKeyBindSlot))
	{
		ensureMsgf(false, TEXT("Invalid key bind slot provided!"));
		return false;
	}

	if (MappableOptions[InKeyBindSlot].InputMapping.Key == NewKey)
	{
		return false;
	}

	UBELocalPlayer* BELocalPlayer = CastChecked<UBELocalPlayer>(LocalPlayer);
	UBESettingsLocal* LocalSettings = BELocalPlayer->GetLocalSettings();

	LocalSettings->AddOrUpdateCustomKeyboardBindings(MappableOptions[InKeyBindSlot].InputMapping.PlayerMappableOptions.Name, NewKey, BELocalPlayer);
	MappableOptions[InKeyBindSlot].InputMapping.Key = NewKey;

	NotifySettingChanged(EGameSettingChangeReason::Change);

	return true;
}

void UBESettingValueKeyConfig::GetAllMappedActionsFromKey(int32 InKeyBindSlot, FKey Key, TArray<FName>& OutActionNames) const
{
	if (MappableOptions.IsValidIndex(InKeyBindSlot))
	{
		if (MappableOptions[InKeyBindSlot].InputMapping.Key == Key)
		{
			return;
		}
	}
	
	if (const UBELocalPlayer* BELocalPlayer = CastChecked<UBELocalPlayer>(LocalPlayer))
	{
		UBESettingsLocal* LocalSettings = BELocalPlayer->GetLocalSettings();
		LocalSettings->GetAllMappingNamesFromKey(Key, OutActionNames);
	}
}

#undef LOCTEXT_NAMESPACE
