// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "Widgets/GameSettingScreen.h"

#include "Engine/DataTable.h"
#include "Input/UIActionBindingHandle.h"
#include "UObject/UObjectGlobals.h"

#include "BESettingScreen.generated.h"

class UBETabListWidgetBase;
class UGameSettingRegistry;
class UObject;


UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class UBESettingScreen : public UGameSettingScreen
{
	GENERATED_BODY()
protected:
	virtual void NativeOnInitialized() override;
	virtual UGameSettingRegistry* CreateRegistry() override;

	void HandleBackAction();
	void HandleApplyAction();
	void HandleCancelChangesAction();

	virtual void OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty) override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = Input, meta = (BindWidget, OptionalWidget = true, AllowPrivateAccess = true))
	UBETabListWidgetBase* TopSettingsTabs;
	
	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle BackInputActionData;

	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle ApplyInputActionData;

	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle CancelChangesInputActionData;

	FUIActionBindingHandle BackHandle;
	FUIActionBindingHandle ApplyHandle;
	FUIActionBindingHandle CancelChangesHandle;
};
