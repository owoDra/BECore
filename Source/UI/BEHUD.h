// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "GameFramework/HUD.h"

#include "Containers/Array.h"
#include "Engine/EngineTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BEHUD.generated.h"

class AActor;
class UObject;


/**
 * ABEHUD
 *
 *  Note that you typically do not need to extend or modify this class, instead you would
 *  use an "Add Widget" action in your experience to add a HUD layout and widgets to it
 * 
 *  This class exists primarily for debug rendering
 */
UCLASS(Config = Game)
class ABEHUD : public AHUD
{
	GENERATED_BODY()

public:
	ABEHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	//~UObject interface
	virtual void PreInitializeComponents() override;
	//~End of UObject interface

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AHUD interface
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
	//~End of AHUD interface
};
