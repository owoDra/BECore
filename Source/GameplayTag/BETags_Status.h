// Copyright owoDra

#pragma once

#include "NativeGameplayTags.h"


////////////////////////////////////
// Status.LocomotionMode

BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_OnGround);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_InAir);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_InWater);


////////////////////////////////////
// Status.RotationMode

BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_VelocityDirection);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_ViewDirection);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_Aiming);


////////////////////////////////////
// Status.Stance

BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stance_Standing);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stance_Crouching);


////////////////////////////////////
// Status.Gait

BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Walking);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Running);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Sprinting);


////////////////////////////////////
// Status

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Spawning);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Attacking);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dying);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dead);
