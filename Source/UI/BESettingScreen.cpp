// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BESettingScreen.h"

#include "Player/BELocalPlayer.h"
#include "GameSetting/Registry/BEGameSettingRegistry.h"
#include "Development/BEDeveloperGameSettings.h"

#include "Containers/Array.h"
#include "Delegates/Delegate.h"
#include "Input/CommonUIInputTypes.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BESettingScreen)

class UGameSettingRegistry;


void UBESettingScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BackHandle = RegisterUIActionBinding(FBindUIActionArgs(BackInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleBackAction)));
	ApplyHandle = RegisterUIActionBinding(FBindUIActionArgs(ApplyInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleApplyAction)));
	CancelChangesHandle = RegisterUIActionBinding(FBindUIActionArgs(CancelChangesInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleCancelChangesAction)));
}

UGameSettingRegistry* UBESettingScreen::CreateRegistry()
{
	UBEGameSettingRegistry* NewRegistry = nullptr;
	UClass* RegistryClass = nullptr;

	// DevSetting から Regstry のクラスを取得
	const UBEDeveloperGameSettings* DevSettings = GetDefault<UBEDeveloperGameSettings>();
	check(DevSettings);

	FSoftClassPath RegistryClassPath = DevSettings->SettingRegistryClass;
	if (RegistryClassPath.IsValid())
	{
		RegistryClass = RegistryClassPath.ResolveClass();
		if (!RegistryClass)
		{
			RegistryClass = RegistryClassPath.TryLoadClass<UClass>();
		}
	}

	// Regstry を作成
	if (RegistryClass)
	{
		NewRegistry = NewObject<UBEGameSettingRegistry>(RegistryClass);
	}
	else
	{
		NewRegistry = NewObject<UBEGameSettingRegistry>();
	}

	// Registy を初期化
	if (UBELocalPlayer* LocalPlayer = CastChecked<UBELocalPlayer>(GetOwningLocalPlayer()))
	{
		NewRegistry->Initialize(LocalPlayer);
	}

	return NewRegistry;
}

void UBESettingScreen::HandleBackAction()
{
	if (AttemptToPopNavigation())
	{
		return;
	}

	ApplyChanges();

	DeactivateWidget();
}

void UBESettingScreen::HandleApplyAction()
{
	ApplyChanges();
}

void UBESettingScreen::HandleCancelChangesAction()
{
	CancelChanges();
}

void UBESettingScreen::OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty)
{
	if (bSettingsDirty)
	{
		if (!GetActionBindings().Contains(ApplyHandle))
		{
			AddActionBinding(ApplyHandle);
		}
		if (!GetActionBindings().Contains(CancelChangesHandle))
		{
			AddActionBinding(CancelChangesHandle);
		}
	}
	else
	{
		RemoveActionBinding(ApplyHandle);
		RemoveActionBinding(CancelChangesHandle);
	}
}