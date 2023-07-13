// Copyright owoDra

#pragma once

#include "NativeGameplayTags.h"


////////////////////////////////////
// Status.LocomotionMode

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_OnGround);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_InAir);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_InWater);


////////////////////////////////////
// Status.RotationMode

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_VelocityDirection);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_ViewDirection);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_Aiming);


////////////////////////////////////
// Status.Stance

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stance_Standing);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stance_Crouching);


////////////////////////////////////
// Status.Gait

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Walking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Running);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Sprinting);


////////////////////////////////////
// Status

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Spawning);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Attacking);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dying);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dead);
