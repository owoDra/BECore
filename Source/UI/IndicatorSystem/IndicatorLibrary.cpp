// Copyright Eigi Chin

#include "IndicatorLibrary.h"
#include "BEIndicatorManagerComponent.h"

UIndicatorLibrary::UIndicatorLibrary()
{
}

UBEIndicatorManagerComponent* UIndicatorLibrary::GetIndicatorManagerComponent(AController* Controller)
{
	return UBEIndicatorManagerComponent::GetComponent(Controller);
}
