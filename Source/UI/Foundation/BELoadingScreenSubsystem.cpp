// Copyright owoDra

#include "BELoadingScreenSubsystem.h"

//////////////////////////////////////////////////////////////////////
// UBELoadingScreenSubsystem

UBELoadingScreenSubsystem::UBELoadingScreenSubsystem()
{
}

void UBELoadingScreenSubsystem::SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (LoadingScreenWidgetClass != NewWidgetClass)
	{
		LoadingScreenWidgetClass = NewWidgetClass;

		OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
	}
}

TSubclassOf<UUserWidget> UBELoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
	return LoadingScreenWidgetClass;
}
