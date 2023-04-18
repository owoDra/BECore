// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CommonUserWidget.h"

#include "Components/SlateWrapperTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/UObjectGlobals.h"

#include "BETaggedWidget.generated.h"

class UObject;


/**
 * An widget in a layout that has been tagged (can be hidden or shown via tags on the owning player)
 */
UCLASS(Abstract, Blueprintable)
class UBETaggedWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UBETaggedWidget(const FObjectInitializer& ObjectInitializer);

	//~UWidget interface
	virtual void SetVisibility(ESlateVisibility InVisibility) override;
	//~End of UWidget interface

	//~UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~End of UUserWidget interface

protected:
	/** If the owning player has any of these tags, this widget will be hidden (using HiddenVisibility) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FGameplayTagContainer HiddenByTags;

	/** The visibility to use when this widget is shown (not hidden by gameplay tags). */
	UPROPERTY(EditAnywhere, Category = "HUD")
	ESlateVisibility ShownVisibility = ESlateVisibility::Visible;

	/** The visibility to use when this widget is hidden by gameplay tags. */
	UPROPERTY(EditAnywhere, Category = "HUD")
	ESlateVisibility HiddenVisibility = ESlateVisibility::Collapsed;

	/** Do we want to be visible (ignoring tags)? */
	bool bWantsToBeVisible = true;

private:
	void OnWatchedTagsChanged();
};