// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEEquipmentDefinition.h"
#include "BEEquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEEquipmentDefinition)

UBEEquipmentDefinition::UBEEquipmentDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstanceType = UBEEquipmentInstance::StaticClass();
	EquipWhenAdded = false;
}
