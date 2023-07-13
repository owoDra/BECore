// Copyright owoDra

#include "BETags_GameplayEffect.h"


////////////////////////////////////
// Effect.Type

UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Type_Damage_Instant		, "Effect.Type.Damage.Instant");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Type_Damage_Periodic		, "Effect.Type.Damage.Periodic");

UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Type_Heal_Health_Instant	, "Effect.Type.Heal.Health.Instant");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Type_Heal_Health_Periodic	, "Effect.Type.Heal.Health.Periodic");

UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Type_Heal_Shield_Instant	, "Effect.Type.Heal.Shield.Instant");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_Type_Heal_Shield_Periodic	, "Effect.Type.Heal.Shield.Periodic");


////////////////////////////////////
// SetByCaller

UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Damage		, "SetByCaller.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Heal_Health	, "SetByCaller.Heal.Health");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Heal_Shield	, "SetByCaller.Heal.Shield");
