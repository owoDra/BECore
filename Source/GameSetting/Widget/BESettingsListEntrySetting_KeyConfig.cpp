// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "GameSetting/Widget/BESettingsListEntrySetting_KeyConfig.h"

#include "GameSetting/SettingValue/BESettingValueKeyConfig.h"
#include "UI/Foundation/BEButtonBase.h"
#include "GameplayTag/BETags_UI.h"

#include "CommonActivatableWidget.h"
#include "CommonButtonBase.h"
#include "CommonUIExtensions.h"
#include "Delegates/Delegate.h"
#include "GameSetting.h"
#include "Misc/AssertionMacros.h"
#include "NativeGameplayTags.h"
#include "Templates/Casts.h"
#include "UObject/NameTypes.h"
#include "Widgets/Misc/GameSettingPressAnyKey.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingsListEntrySetting_KeyConfig)

//////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "BESettings"

void UBESettingsListEntrySetting_KeyConfig::SetSetting(UGameSetting* InSetting)
{
	KeyConfigSetting = CastChecked<UBESettingValueKeyConfig>(InSetting);

	Super::SetSetting(InSetting);

	Refresh();
}

void UBESettingsListEntrySetting_KeyConfig::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_AddKey->OnClicked().AddUObject(this, &ThisClass::HandleAddKeyClicked);
}

void UBESettingsListEntrySetting_KeyConfig::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	KeyConfigSetting = nullptr;
}

void UBESettingsListEntrySetting_KeyConfig::OnSettingChanged()
{
	Refresh();
}

void UBESettingsListEntrySetting_KeyConfig::ChangeBinding(int32 BindSlot, FKey InKey)
{
	OriginalKeyToBind = InKey;

	TArray<FName> ActionsForKey;
	KeyConfigSetting->GetAllMappedActionsFromKey(BindSlot, InKey, ActionsForKey);

	// すでにキーが割り当てられている
	if (!ActionsForKey.IsEmpty())
	{
		UKeyAlreadyBoundWarning* KeyAlreadyBoundWarning = CastChecked<UKeyAlreadyBoundWarning>(
			UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(), TAG_UI_Layer_Modal, KeyAlreadyBoundWarningPanelClass));

		FString ActionNames;
		for (FName ActionName : ActionsForKey)
		{
			ActionNames += ActionName.ToString() += ", ";
		}

		FFormatNamedArguments Args;
		Args.Add(TEXT("ActionNames"), FText::FromString(ActionNames));

		KeyAlreadyBoundWarning->SetWarningText(FText::Format(LOCTEXT("WarningText", "Selected key is already bound to {ActionNames} are you sure you want to rebind it?"), Args));
		KeyAlreadyBoundWarning->SetCancelText(FText::Format(LOCTEXT("CancelText", "Press escape to cancel, or press key again to confirm rebinding."), Args));

		KeyAlreadyBoundWarning->OnKeySelected.AddUObject(this, &ThisClass::HandleDuplicateKeySelected, KeyAlreadyBoundWarning);
		KeyAlreadyBoundWarning->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, KeyAlreadyBoundWarning);
	}
	else
	{
		KeyConfigSetting->ChangeBinding(BindSlot, InKey);
		Button_AddKey->SetFocus();
	}
}

void UBESettingsListEntrySetting_KeyConfig::Refresh()
{
	TArray<FKey> BoundKeys;
	FindEmptySlot(BoundKeys);
	K2_Refresh(BoundKeys);
}

int32 UBESettingsListEntrySetting_KeyConfig::FindEmptySlot(TArray<FKey>& BoundKeys)
{
	int32 FoundSlot = -1;
	TArray<FKey> NewBoundKey;

	if (KeyConfigSetting)
	{
		TArray<FKeyOption> Options = KeyConfigSetting->GetMappableOptions();
		for (int i = 0; i < Options.Num(); i++)
		{
			if (Options[i].InputMapping.Key == EKeys::Invalid)
			{
				FoundSlot = i;
			}

			NewBoundKey.Add(Options[i].InputMapping.Key);
		}
	}

	BoundKeys = NewBoundKey;

	return FoundSlot;
}

void UBESettingsListEntrySetting_KeyConfig::HandleAddKeyClicked()
{
	if (!PressAnyKeyPanelClass)
	{
		return;
	}

	TArray<FKey> BoundKeys;
	int32 EmptySlot = FindEmptySlot(BoundKeys);
	if (EmptySlot != -1 && EmptySlot < MaxMappableKeys)
	{
		TargetSlotToBind = EmptySlot;

		UGameSettingPressAnyKey* PressAnyKeyPanel = CastChecked<UGameSettingPressAnyKey>(
			UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(), TAG_UI_Layer_Modal, PressAnyKeyPanelClass));

		PressAnyKeyPanel->OnKeySelected.AddUObject(this, &ThisClass::HandleKeySelected, PressAnyKeyPanel);
		PressAnyKeyPanel->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, PressAnyKeyPanel);
	}
}

void UBESettingsListEntrySetting_KeyConfig::HandleKeySelected(FKey InKey, UGameSettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	ChangeBinding(TargetSlotToBind, InKey);
}

void UBESettingsListEntrySetting_KeyConfig::HandleDuplicateKeySelected(FKey InKey, UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const
{
	DuplicateKeyPressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	KeyConfigSetting->ChangeBinding(TargetSlotToBind, OriginalKeyToBind);
	Button_AddKey->SetFocus();
}

void UBESettingsListEntrySetting_KeyConfig::HandleKeySelectionCanceled(UGameSettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelectionCanceled.RemoveAll(this);
	Button_AddKey->SetFocus();
}

void UBESettingsListEntrySetting_KeyConfig::HandleKeySelectionCanceled(UKeyAlreadyBoundWarning* AlreadyBoundWarningPanel)
{
	AlreadyBoundWarningPanel->OnKeySelectionCanceled.RemoveAll(this);
	Button_AddKey->SetFocus();
}

#undef LOCTEXT_NAMESPACE
