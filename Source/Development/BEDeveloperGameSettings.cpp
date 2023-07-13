// Copyright owoDra

#include "BEDeveloperGameSettings.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEDeveloperGameSettings)

#define LOCTEXT_NAMESPACE "BECheats"


UBEDeveloperGameSettings::UBEDeveloperGameSettings()
{
}


FName UBEDeveloperGameSettings::GetCategoryName() const
{
	return FName(FString::Printf(TEXT("%s: Game"), FApp::GetProjectName()));
}


#if WITH_EDITOR

void UBEDeveloperGameSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UBEDeveloperGameSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);
}

void UBEDeveloperGameSettings::PostInitProperties()
{
	Super::PostInitProperties();
}


void UBEDeveloperGameSettings::OnPlayInEditorStarted() const
{
	// Show a notification toast to remind the user that there's an experience override set
	if (ExperienceOverride.IsValid())
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("ExperienceOverrideActive", "Developer Settings Override\nExperience {0}"),
			FText::FromName(ExperienceOverride.PrimaryAssetName)
		));
		Info.ExpireDuration = 2.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

#endif

#undef LOCTEXT_NAMESPACE
