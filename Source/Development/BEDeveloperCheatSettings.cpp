// Copyright owoDra

#include "BEDeveloperCheatSettings.h"

#include "Misc/App.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEDeveloperCheatSettings)


#define LOCTEXT_NAMESPACE "BECheat"

UBEDeveloperCheatSettings::UBEDeveloperCheatSettings()
{
}


FName UBEDeveloperCheatSettings::GetCategoryName() const
{
	return FName(FString::Printf(TEXT("%s: Cheat"), FApp::GetProjectName()));
}


#if WITH_EDITOR

void UBEDeveloperCheatSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UBEDeveloperCheatSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);
}

void UBEDeveloperCheatSettings::PostInitProperties()
{
	Super::PostInitProperties();
}

#endif

#undef LOCTEXT_NAMESPACE
