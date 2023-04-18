// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CommonGameViewportClient.h"

#include "UObject/UObjectGlobals.h"

#include "BEGameViewportClient.generated.h"

class UGameInstance;
class UObject;


UCLASS(BlueprintType)
class UBEGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	UBEGameViewportClient();

	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};