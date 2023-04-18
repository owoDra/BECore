// Copyright Eigi Chin

#pragma once

#include "Components/ControllerComponent.h"
#include "Delegates/DelegateCombinations.h"

#include "BEIndicatorManagerComponent.generated.h"

class UIndicatorDescriptor;

/**
 * UBEIndicatorManagerComponent
 */
UCLASS(BlueprintType, Blueprintable)
class BECORE_API UBEIndicatorManagerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UBEIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer);

	static UBEIndicatorManagerComponent* GetComponent(AController* Controller);

	UFUNCTION(BlueprintCallable, Category = Indicator)
	void AddIndicator(UIndicatorDescriptor* IndicatorDescriptor);
	
	UFUNCTION(BlueprintCallable, Category = Indicator)
	void RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor);

	DECLARE_EVENT_OneParam(UBEIndicatorManagerComponent, FIndicatorEvent, UIndicatorDescriptor* Descriptor)
	FIndicatorEvent OnIndicatorAdded;
	FIndicatorEvent OnIndicatorRemoved;

	const TArray<UIndicatorDescriptor*>& GetIndicators() const { return Indicators; }

private:
	UPROPERTY()
	TArray<UIndicatorDescriptor*> Indicators;
};
