// Copyright owoDra

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectMacros.h"
#include "BEWidgetFactory.h"

#include "BEWidgetFactory_Class.generated.h"

UCLASS()
class BECORE_API UBEWidgetFactory_Class : public UBEWidgetFactory
{
	GENERATED_BODY()

public:
	UBEWidgetFactory_Class() { }

	virtual TSubclassOf<UUserWidget> FindWidgetClassForData_Implementation(const UObject* Data) const override;
	
protected:
	UPROPERTY(EditAnywhere, Category = ListEntries, meta = (AllowAbstract))
	TMap<TSoftClassPtr<UObject>, TSubclassOf<UUserWidget>> EntryWidgetForClass;
};