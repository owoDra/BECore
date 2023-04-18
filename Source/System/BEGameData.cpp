// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameData.h"
#include "BEAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEGameData)

/////////////////////////////////////////////////

UBEGameData::UBEGameData()
{
}

const UBEGameData& UBEGameData::UBEGameData::Get()
{
	return UBEAssetManager::Get().GetGameData();
}