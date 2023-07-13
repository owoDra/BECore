// Copyright owoDra

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "UObject/ScriptInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IndicatorLibrary.generated.h"

class UBEIndicatorManagerComponent;

UCLASS()
class BECORE_API UIndicatorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UIndicatorLibrary();
	
	/**  */
	UFUNCTION(BlueprintCallable, Category = Indicator)
	static UBEIndicatorManagerComponent* GetIndicatorManagerComponent(AController* Controller);
};
