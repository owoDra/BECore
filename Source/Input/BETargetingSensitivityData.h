// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Engine/DataAsset.h"

#include "GameSetting/BEGameSharedSettings.h"

#include "Containers/Map.h"
#include "UObject/UObjectGlobals.h"

#include "BETargetingSensitivityData.generated.h"

class UObject;


/** Defines a set of gamepad sensitivity to a float value. */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "BE Aim Sensitivity Data", ShortTooltip = "Data asset used to define a map of Gamepad Sensitivty to a float value."))
class BECORE_API UBETargetingSensitivityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBETargetingSensitivityData(const FObjectInitializer& ObjectInitializer);
	
	const float SensitivtyEnumToFloat(const EBEGamepadSensitivity InSensitivity) const;
	
protected:
	/** Map of SensitivityMap settings to their corresponding float */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EBEGamepadSensitivity, float> SensitivityMap;
};