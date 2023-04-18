// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Internationalization/PolyglotTextData.h"
#include "BETextHotfixConfig.generated.h"


/**
 * This class allows hotfixing individual FText values anywhere
 */

UCLASS(config=Game, defaultconfig)
class UBETextHotfixConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBETextHotfixConfig(const FObjectInitializer& ObjectInitializer);

	// UObject interface
	virtual void PostInitProperties() override;
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End of UObject interface

private:
	void ApplyTextReplacements() const;

private:
	// The list of FText values to hotfix
	UPROPERTY(Config, EditAnywhere)
	TArray<FPolyglotTextData> TextReplacements;
};
