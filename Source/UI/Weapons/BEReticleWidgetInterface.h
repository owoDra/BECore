// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "BEReticleWidgetInterface.generated.h"

class UBEWeaponInstance;
class UBEInventoryItemInstance;

//////////////////////////////////////////////

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UBEReticleWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

//////////////////////////////////////////////

class BECORE_API IBEReticleWidgetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Reticle")
		void InitializeFromWeapon(UBEWeaponInstance* InWeapon, UBEInventoryItemInstance* InItem);
};
