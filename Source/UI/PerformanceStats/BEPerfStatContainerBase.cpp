// Copyright Eigi Chin

#include "BEPerfStatContainerBase.h"
#include "GameSetting/BEGameDeviceSettings.h"
#include "BEPerfStatWidgetBase.h"
#include "Blueprint/WidgetTree.h"

//////////////////////////////////////////////////////////////////////
// UBEPerfStatsContainerBase

UBEPerfStatContainerBase::UBEPerfStatContainerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBEPerfStatContainerBase::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateVisibilityOfChildren();

	UBEGameDeviceSettings::Get()->OnPerfStatSettingsChangedEvent.AddUObject(this, &ThisClass::UpdateVisibilityOfChildren);
}

void UBEPerfStatContainerBase::NativeDestruct()
{
	UBEGameDeviceSettings::Get()->OnPerfStatSettingsChangedEvent.RemoveAll(this);

	Super::NativeDestruct();
}

void UBEPerfStatContainerBase::UpdateVisibilityOfChildren()
{
	UBEGameDeviceSettings* UserSettings = UBEGameDeviceSettings::Get();

	const bool bShowTextWidgets = (StatDisplayModeFilter == EBEStatDisplayMode::TextOnly) || (StatDisplayModeFilter == EBEStatDisplayMode::TextAndGraph);
	const bool bShowGraphWidgets = (StatDisplayModeFilter == EBEStatDisplayMode::GraphOnly) || (StatDisplayModeFilter == EBEStatDisplayMode::TextAndGraph);
	
	check(WidgetTree);
	WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		if (UBEPerfStatWidgetBase* TypedWidget = Cast<UBEPerfStatWidgetBase>(Widget))
		{
			const EBEStatDisplayMode SettingMode = UserSettings->GetPerfStatDisplayState(TypedWidget->GetStatToDisplay());

			bool bShowWidget = false;
			switch (SettingMode)
			{
			case EBEStatDisplayMode::Hidden:
				bShowWidget = false;
				break;
			case EBEStatDisplayMode::TextOnly:
				bShowWidget = bShowTextWidgets;
				break;
			case EBEStatDisplayMode::GraphOnly:
				bShowWidget = bShowGraphWidgets;
				break;
			case EBEStatDisplayMode::TextAndGraph:
				bShowWidget = bShowTextWidgets || bShowGraphWidgets;
				break;
			}

			TypedWidget->SetVisibility(bShowWidget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	});
}
