// Copyright Eigi Chin

#include "BEPerfStatContainerBase.h"
#include "Setting/BESettingsLocal.h"
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

	UBESettingsLocal::Get()->OnPerfStatSettingsChangedEvent.AddUObject(this, &ThisClass::UpdateVisibilityOfChildren);
}

void UBEPerfStatContainerBase::NativeDestruct()
{
	UBESettingsLocal::Get()->OnPerfStatSettingsChangedEvent.RemoveAll(this);

	Super::NativeDestruct();
}

void UBEPerfStatContainerBase::UpdateVisibilityOfChildren()
{
	UBESettingsLocal* UserSettings = UBESettingsLocal::Get();

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
