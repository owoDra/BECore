// Copyright owoDra

#include "BEWidgetFactory.h"

TSubclassOf<UUserWidget> UBEWidgetFactory::FindWidgetClassForData_Implementation(const UObject* Data) const
{
	return TSubclassOf<UUserWidget>();
}