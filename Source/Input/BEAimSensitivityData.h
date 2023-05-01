// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "Setting/BESettingsShared.h"

#include "Containers/Map.h"
#include "UObject/UObjectGlobals.h"

#include "BEAimSensitivityData.generated.h"

class UObject;


/** Defines a set of gamepad sensitivity to a float value. */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "BE Aim Sensitivity Data", ShortTooltip = "Data asset used to define a map of Gamepad Sensitivty to a float value."))
class BECORE_API UBEAimSensitivityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBEAimSensitivityData(const FObjectInitializer& ObjectInitializer);
	
	const float SensitivtyEnumToFloat(const EBEGamepadSensitivity InSensitivity) const;
	
protected:
	/** Map of SensitivityMap settings to their corresponding float */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EBEGamepadSensitivity, float> SensitivityMap;
};