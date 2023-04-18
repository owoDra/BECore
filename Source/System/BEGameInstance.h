// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CommonGameInstance.h"

#include "UObject/UObjectGlobals.h"

#include "BEGameInstance.generated.h"

class ABEPlayerController;
class UObject;


UCLASS(Config = Game)
class BECORE_API UBEGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:

	UBEGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	ABEPlayerController* GetPrimaryPlayerController() const;

	virtual bool CanJoinRequestedSession() const override;
protected:

	virtual void Init() override;
	virtual void Shutdown() override;
};
