// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BETextHotfixConfig.h"
#include "Internationalization/TextLocalizationManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BETextHotfixConfig)


UBETextHotfixConfig::UBETextHotfixConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBETextHotfixConfig::ApplyTextReplacements() const
{
	FTextLocalizationManager::Get().RegisterPolyglotTextData(TextReplacements);
}

void UBETextHotfixConfig::PostInitProperties()
{
	Super::PostInitProperties();
	ApplyTextReplacements();
}

void UBETextHotfixConfig::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);
	ApplyTextReplacements();
}

#if WITH_EDITOR
void UBETextHotfixConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplyTextReplacements();
}
#endif
