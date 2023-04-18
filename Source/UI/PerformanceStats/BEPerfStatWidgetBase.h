// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"

#include "BEPerfStatWidgetBase.generated.h"

class UBEPerformanceStatSubsystem;

/**
 * UBEPerfStatWidgetBase
 *
 * Base class for a widget that displays a single stat, e.g., FPS, ping, etc...
 */
 UCLASS(Abstract)
class UBEPerfStatWidgetBase : public UCommonUserWidget
{
public:
	UBEPerfStatWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	GENERATED_BODY()

public:
	// Returns the stat this widget is supposed to display
	UFUNCTION(BlueprintPure)
	EBEDisplayablePerformanceStat GetStatToDisplay() const
	{
		return StatToDisplay;
	}

	// Polls for the value of this stat (unscaled)
	UFUNCTION(BlueprintPure)
	double FetchStatValue();

protected:
	// Cached subsystem pointer
	UPROPERTY(Transient)
	TObjectPtr<UBEPerformanceStatSubsystem> CachedStatSubsystem;

	// The stat to display
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Display)
	EBEDisplayablePerformanceStat StatToDisplay;
 };
