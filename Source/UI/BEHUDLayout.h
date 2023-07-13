// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "BEActivatableWidget.h"

#include "UObject/SoftObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEHUDLayout.generated.h"

class UCommonActivatableWidget;
class UObject;


/**
 * UBEHUDLayout
 *
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "BE HUD Layout", Category = "BE|HUD"))
class UBEHUDLayout : public UBEActivatableWidget
{
	GENERATED_BODY()

public:
	UBEHUDLayout(const FObjectInitializer& ObjectInitializer);

	void NativeOnInitialized() override;

protected:
	void HandleEscapeAction();

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;
};
