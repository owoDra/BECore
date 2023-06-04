// Copyright Eigi Chin

#pragma once

#include "NativeGameplayTags.h"


///////////////////////////////////////////////////////
// Ability.ActivateFail

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_IsDead);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_Cooldown);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_TagsBlocked);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_TagsMissing);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_Networking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_ActivationGroup);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_Cost);


///////////////////////////////////////////////////////
// Ability.Behavior

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Behavior_SurvivesDeath);


///////////////////////////////////////////////////////
// Ability.Type

BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Movement);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Action);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Passive);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Skill);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Weapon);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_StatusChange);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Misc);


///////////////////////////////////////////////////////
// Ability.Trigger

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Trigger_Reset);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Trigger_Attack);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Trigger_AltAttack);
