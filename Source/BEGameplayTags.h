// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"

class UGameplayTagsManager;


/// =================================
///  ゲームプレイタグ定義
/// =================================

// =================================
//  アビリティ
// =================================




////////////////////////////////////
// タイプ

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Action_Crouch);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Action_Run);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Action_Jump);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Action_Interact);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Action_Tactical);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Action_Weapon);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Passive_AutoRespawn);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Passive_ChangeQuickBarSlot);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_StatusChange_Death);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_StatusChange_Spawning);


// =================================
//  メッセージ
// =================================

////////////////////////////////////
// アビリティ

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Ability_UserFacingSimpleActivateFail);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Ability_PlayMontageOnActivateFail);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Ability_Interaction_Duration);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Ability_Tactical_Duration);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Ability_Tactical_Recharge);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Ability_Tactical_Activate);


////////////////////////////////////
// ダメージ

BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Damage);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Elimination);
BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Assist);


////////////////////////////////////
// クイックバー

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_QuickBar_SlotsChanged);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_QuickBar_ActiveIndexChanged);


////////////////////////////////////
// 通知

BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_AddNotification);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Accolade);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_ElimFeed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Reset);


// =================================
//  入力
// =================================



UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_Crouch);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_Run);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_Jump);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_Interact);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_Tactical);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_Weapon);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_QuickSlot_Next);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_QuickSlot_Back);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_QuickSlot_Select);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_QuickSlot_Drop);


// =================================
//  イベント
// =================================

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Death);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_RequestReset);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_InteractionActivate);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_AbilityRecharge);


// =================================
//  ゲームプレイ
// =================================

////////////////////////////////////
// ダメージ

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage_SelfDestruct);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage_FellOutOfWorld);


////////////////////////////////////
// 武器

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Zone);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Zone_WeakSpot);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Zone_StrongSpot);


////////////////////////////////////
// アロケード

BECORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Accolade);


// =================================
//  ゲームプレイエフェクト
// =================================

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_DamageTrait_Instant);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_DamageTrait_Periodic);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_HealTrait_Health_Instant);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_HealTrait_Health_Periodic);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_HealTrait_Shield_Instant);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_HealTrait_Shield_Periodic);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_DamageType_Basic);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_DamageType_Environment);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_DamageType_Weapon);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEffect_DamageType_Ability);


// =================================
//  Stat
// =================================

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Weapon);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Ability_Tactical_StockSize);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Ability_Tactical_Stock);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Ability_Tactical_ReqElimination);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Ability_Tactical_Elimination);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Ability_Tactical_RechargeTime);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Player_Eliminate);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Player_Assist);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Player_Down);


// =================================
//  UI
// =================================

////////////////////////////////////
// レイヤー

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Layer_Modal);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Layer_Menu);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Layer_Game);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Layer_GameMenu);


////////////////////////////////////
// アクション

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Action_Escape);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Action_Back);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Action_Cancel);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Action_Confirm);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Action_NextTab);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_Action_PreviousTab);


// =================================
//  HUD
// =================================

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_HUD_Slot);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_HUD_Slot_PerfStats_Graph);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_HUD_Slot_PerfStats_Text);


// =================================
//  カメラ
// =================================

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Camera_Type_FirstPerson);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Camera_Type_ThirdPerson);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Camera_Type_Death);


// =================================
//  SetByCaller
// =================================

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SetByCaller_Damage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SetByCaller_Heal_Health);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SetByCaller_Heal_Shield);


// =================================
//  チート
// =================================

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cheat_UnlimitedHealth);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cheat_UnlimitedAmmo);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cheat_UnlimitedAbility);


// =================================
//  コスメティック
// =================================

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cosmetic);


// =================================
//  状態
// =================================

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_IsPlayer);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Spawning);



UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_AbilityInputBlocked);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_WeaponFireBlocked);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_WeaponAimBlocked);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_JumpBlocked);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RunBlocked);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_SprintBlocked);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_MovementStopped);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_DamageImmunity);


/// =================================
///  構造多
/// =================================

/**
 * FBEGameplayTags
 *
 * Singleton containing native gameplay tags.
 */
struct BECORE_API FBEGameplayTags
{
public:

	static const FBEGameplayTags& Get() { return GameplayTags; }

	static void InitializeNativeTags();

	static FGameplayTag FindTagByString(FString TagString, bool bMatchPartialString = false);

public:

	// =================================
	//  移動状態
	// =================================

	FGameplayTag Movement_Mode_Walking;
	FGameplayTag Movement_Mode_NavWalking;
	FGameplayTag Movement_Mode_Falling;
	FGameplayTag Movement_Mode_Swimming;
	FGameplayTag Movement_Mode_Flying;
	FGameplayTag Movement_Mode_Custom;

	FGameplayTag CustomMovement_Mode_Slide;
	FGameplayTag CustomMovement_Mode_Climb;
	FGameplayTag CustomMovement_Mode_WallRunR;
	FGameplayTag CustomMovement_Mode_WallRunL;

	TMap<uint8, FGameplayTag> MovementModeTagMap;
	TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

protected:

	void AddAllTags(UGameplayTagsManager& Manager);
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);
	void AddMovementModeTag(FGameplayTag& OutTag, const ANSICHAR* TagName, uint8 MovementMode);
	void AddCustomMovementModeTag(FGameplayTag& OutTag, const ANSICHAR* TagName, uint8 CustomMovementMode);

private:

	static FBEGameplayTags GameplayTags;
};
