﻿// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BEExperienceDefinition.generated.h"

class UGameFeatureAction;
class UBEPawnData;
class UBEExperienceActionSet;


/**
 * Definition of an experience
 */
UCLASS(BlueprintType, Const, Abstract)
class UBEExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBEExperienceDefinition();

	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
	//~End of UObject interface

	//~UPrimaryDataAsset interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~End of UPrimaryDataAsset interface

public:
	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;

	/** The default pawn class to spawn for players */
	//@TODO: Make soft?
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const UBEPawnData> DefaultPawnData;

	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// List of additional action sets to compose into this experience
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<TObjectPtr<UBEExperienceActionSet>> ActionSets;
};
