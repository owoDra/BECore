﻿// Copyright Eigi Chin

#include "BEPawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPawnData)


UBEPawnData::UBEPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnClass			= nullptr;
	InputConfig			= nullptr;
	DefaultCameraMode	= nullptr;
	EquipmentSet		= nullptr;
	DefaultTPPAnimLayer = nullptr;
	DefaultFPPAnimLayer = nullptr;
}
