// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Performance/BEPerformanceStatTypes.h"

#include "BEPerfStatContainerBase.generated.h"

/**
 * UBEPerfStatsContainerBase
 *
 * Panel that contains a set of UBEPerfStatWidgetBase widgets and manages
 * their visibility based on user settings.
 */
 UCLASS(Abstract)
class UBEPerfStatContainerBase : public UCommonUserWidget
{
public:
	UBEPerfStatContainerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	GENERATED_BODY()

	//~UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~End of UUserWidget interface

	UFUNCTION(BlueprintCallable)
	void UpdateVisibilityOfChildren();

protected:
	// Are we showing text or graph stats?
	UPROPERTY(EditAnywhere, Category=Display)
	EBEStatDisplayMode StatDisplayModeFilter = EBEStatDisplayMode::TextAndGraph;
};
