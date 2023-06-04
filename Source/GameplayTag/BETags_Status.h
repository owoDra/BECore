// Copyright Eigi Chin

#pragma once

#include "NativeGameplayTags.h"


////////////////////////////////////
// Status.MoveMode

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_Walking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_NavWalking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_Falling);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_Swimming);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_Flying);

////////////////////////////////////
// Status

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Crouching);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Sprinting);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Targeting);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Spawning);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Attacking);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dying);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dead);
