// Copyright owoDra

#include "BECharacterMovementData.h"

#include "GameplayTag/BETags_Status.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterMovementData)


UBECharacterMovementData::UBECharacterMovementData()
{
	LocomotionModes =
	{
		{ TAG_Status_LocomotionMode_OnGround, FBECharacterLocomotionModeConfigs() },
		{ TAG_Status_LocomotionMode_InAir	, FBECharacterLocomotionModeConfigs() },
		{ TAG_Status_LocomotionMode_InWater	, FBECharacterLocomotionModeConfigs() }
	};
}
