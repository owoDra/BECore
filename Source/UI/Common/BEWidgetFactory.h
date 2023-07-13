// Copyright owoDra

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectMacros.h"

#include "BEWidgetFactory.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew)
class BECORE_API UBEWidgetFactory : public UObject
{
	GENERATED_BODY()

public:
	UBEWidgetFactory() { }

	UFUNCTION(BlueprintNativeEvent)
	TSubclassOf<UUserWidget> FindWidgetClassForData(const UObject* Data) const;
};