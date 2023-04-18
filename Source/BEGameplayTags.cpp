// Copyright Eigi Chin

#include "BEGameplayTags.h"
#include "BELogChannels.h"
#include "GameplayTagsManager.h"
#include "Engine/EngineTypes.h"

FBEGameplayTags FBEGameplayTags::GameplayTags;


/// =================================
///  ゲームプレイタグ定義
/// =================================

// =================================
//  アビリティ
// =================================

////////////////////////////////////
// 実行失敗

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup");

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_ActivateFail_MagazineFull, "Ability.ActivateFail.MagazineFull");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_ActivateFail_NoAmmo, "Ability.ActivateFail.NoAmmo");


////////////////////////////////////
// ビヘイビアー

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath");


////////////////////////////////////
// タイプ

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Crouch, "Ability.Type.Action.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Run, "Ability.Type.Action.Run");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Jump, "Ability.Type.Action.Jump");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Mantle, "Ability.Type.Action.Mantle");

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Interact, "Ability.Type.Action.Interact");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_ShowOverlay, "Ability.Type.Action.ShowOverlay");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Ping, "Ability.Type.Action.Ping");

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Tactical_Main, "Ability.Type.Action.Tactical.Main");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Tactical_Sub, "Ability.Type.Action.Tactical.Sub");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Tactical_Ultimate, "Ability.Type.Action.Tactical.Ultimate");

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Weapon_Fire, "Ability.Type.Action.Weapon.Fire");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Weapon_AltFire, "Ability.Type.Action.Weapon.AltFire");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Action_Weapon_Reload, "Ability.Type.Action.Weapon.Reload");

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Passive_AutoRespawn, "Ability.Type.Passive.AutoRespawn");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_Passive_ChangeQuickBarSlot, "Ability.Type.Passive.ChangeQuickBarSlot");

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_StatusChange_Death, "Ability.Type.StatusChange.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Type_StatusChange_Spawning, "Ability.Type.StatusChange.Spawning");


// =================================
//  メッセージ
// =================================

////////////////////////////////////
// アビリティ

UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Ability_UserFacingSimpleActivateFail, "Message.Ability.UserFacingSimpleActivateFail");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Ability_PlayMontageOnActivateFail, "Message.Ability.PlayMontageOnActivateFail");

UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Ability_Interaction_Duration, "Message.Ability.Interaction.Duration");

UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Ability_Tactical_Duration, "Message.Ability.Tactical.Duration");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Ability_Tactical_Recharge, "Message.Ability.Tactical.Recharge");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Ability_Tactical_Activate, "Message.Ability.Tactical.Activate");


////////////////////////////////////
// ダメージ

UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Damage, "Message.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Elimination, "Message.Elimination");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Assist, "Message.Assist");


////////////////////////////////////
// クイックバー

UE_DEFINE_GAMEPLAY_TAG(TAG_Message_QuickBar_SlotsChanged, "Message.QuickBar.SlotsChanged");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_QuickBar_ActiveIndexChanged, "Message.QuickBar.ActiveIndexChanged");


////////////////////////////////////
// 通知

UE_DEFINE_GAMEPLAY_TAG(TAG_Message_AddNotification, "Message.AddNotification");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Accolade, "Message.Accolade");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_ElimFeed, "Message.ElimFeed");
UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Reset, "Message.Reset");


// =================================
//  入力
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Move_KM, "Input.Move.KM");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Move_Pad, "Input.Move.Pad");

UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Look_KM, "Input.Look.KM");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Look_Pad, "Input.Look.Pad");

UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Crouch, "Input.Ability.Crouch");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Run, "Input.Ability.Run");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Jump, "Input.Ability.Jump");

UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Interact, "Input.Ability.Interact");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_ShowOverlay, "Input.Ability.ShowOverlay");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Ping, "Input.Ability.Ping");

UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Tactical_Main, "Input.Ability.Tactical.Main");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Tactical_Sub, "Input.Ability.Tactical.Sub");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Tactical_Ultimate, "Input.Ability.Tactical.Ultimate");

UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_QuickSlot_Next, "Input.Ability.QuickSlot.Next");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_QuickSlot_Back, "Input.Ability.QuickSlot.Back");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_QuickSlot_Select, "Input.Ability.QuickSlot.Select");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_QuickSlot_Drop, "Input.Ability.QuickSlot.Drop");

UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Weapon_Fire, "Input.Ability.Weapon.Fire");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Weapon_AltFire, "Input.Ability.Weapon.AltFire");
UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Ability_Weapon_Reload, "Input.Ability.Weapon.Reload");


// =================================
//  イベント
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Death, "Event.Death");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_RequestReset, "Event.RequestReset");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_AltFire, "Event.AltFire");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Melee, "Event.Melee");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_MeleeHit, "Event.MeleeHit");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Reload, "Event.Reload");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_ReloadDone, "Event.ReloadDone");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_WeaponFire, "Event.WeaponFire");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_InteractionActivate, "Event.InteractionActivate");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_AbilityRecharge, "Event.AbilityRecharge");


// =================================
//  ゲームプレイ
// =================================

////////////////////////////////////
// ダメージ

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Damage_SelfDestruct, "Gameplay.Damage.SelfDestruct");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Damage_FellOutOfWorld, "Gameplay.Damage.FellOutOfWorld");


////////////////////////////////////
// 武器

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Zone, "Gameplay.Zone");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Zone_WeakSpot, "Gameplay.Zone.WeakSpot");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Zone_StrongSpot, "Gameplay.Zone.StrongSpot");


////////////////////////////////////
// アロケード

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Accolade, "Gameplay.Accolade");


// =================================
//  ゲームプレイエフェクト
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_DamageTrait_Instant, "GameplayEffect.DamageTrait.Instant");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_DamageTrait_Periodic, "GameplayEffect.DamageTrait.Periodic");

UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_HealTrait_Health_Instant, "GameplayEffect.HealTrait.Health.Instant");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_HealTrait_Health_Periodic, "GameplayEffect.HealTrait.Health.Periodic");

UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_HealTrait_Shield_Instant, "GameplayEffect.HealTrait.Shield.Instant");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_HealTrait_Shield_Periodic, "GameplayEffect.HealTrait.Shield.Periodic");

UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_DamageType_Basic, "GameplayEffect.DamageType.Basic");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_DamageType_Environment, "GameplayEffect.DamageType.Environment");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_DamageType_Weapon, "GameplayEffect.DamageType.Weapon");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameplayEffect_DamageType_Ability, "GameplayEffect.DamageType.Ability");


// =================================
//  Stat
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Weapon_MagazineSize, "Stat.Weapon.MagazineSize");
UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Weapon_MagazineAmmo, "Stat.Weapon.MagazineAmmo");
UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Weapon_SpareAmmo, "Stat.Weapon.SpareAmmo");

UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Ability_Tactical_StockSize, "Stat.Ability.Tactical.StockSize");
UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Ability_Tactical_Stock, "Stat.Ability.Tactical.Stock");
UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Ability_Tactical_ReqElimination, "Stat.Ability.Tactical.ReqElimination");
UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Ability_Tactical_Elimination, "Stat.Ability.Tactical.Elimination");
UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Ability_Tactical_RechargeTime, "Stat.Ability.Tactical.RechargeTime");

UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Ability_Ultimate_UltimatePoint, "Stat.Ability.Ultimate.UltimatePoint");

UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Player_UltimatePoint, "Stat.Player.UltimatePoint");
UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Player_Eliminate, "Stat.Player.Eliminate");
UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Player_Assist, "Stat.Player.Assist");
UE_DEFINE_GAMEPLAY_TAG(TAG_Stat_Player_Down, "Stat.Player.Down");


// =================================
//  UI
// =================================

////////////////////////////////////
// レイヤー

UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Modal, "UI.Layer.Modal");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Menu, "UI.Layer.Menu");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_Game, "UI.Layer.Game");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Layer_GameMenu, "UI.Layer.GameMenu");


////////////////////////////////////
// アクション

UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Action_Escape, "UI.Action.Escape");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Action_Back, "UI.Action.Back");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Action_Cancel, "UI.Action.Cancel");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Action_Confirm, "UI.Action.Confirm");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Action_NextTab, "UI.Action.NextTab");
UE_DEFINE_GAMEPLAY_TAG(TAG_UI_Action_PreviousTab, "UI.Action.PreviousTab");


// =================================
//  HUD
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_EliminationFeed, "HUD.Slot.EliminationFeed");
UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_Equipment, "HUD.Slot.Equipment");
UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_Health, "HUD.Slot.Health");
UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_ModeStatus, "HUD.Slot.ModeStatus");
UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_Reticle, "HUD.Slot.Reticle");
UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_Score, "HUD.Slot.Score");
UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_Accolade, "HUD.Slot.Accolade");
UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_Minimap, "HUD.Slot.Minimap");

UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_PerfStats_Graph, "HUD.Slot.PerfStats.Graph");
UE_DEFINE_GAMEPLAY_TAG(TAG_HUD_Slot_PerfStats_Text, "HUD.Slot.PerfStats.Text");


// =================================
//  カメラ
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_Camera_Type_FirstPerson, "Camera.Type.FirstPerson");
UE_DEFINE_GAMEPLAY_TAG(TAG_Camera_Type_ThirdPerson, "Camera.Type.ThirdPerson");
UE_DEFINE_GAMEPLAY_TAG(TAG_Camera_Type_Death, "Camera.Type.Death");


// =================================
//  プラットフォーム
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_SupportsChangingAudioOutputDevice, "Platform.Trait.SupportsChangingAudioOutputDevice");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_SupportsBackgroundAudio, "Platform.Trait.SupportsBackgroundAudio");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_SupportsWindowedMode, "Platform.Trait.SupportsWindowedMode");

UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_NeedsBrightnessAdjustment, "Platform.Trait.NeedsBrightnessAdjustment");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_BinauralSettingControlledByOS, "Platform.Trait.BinauralSettingControlledByOS");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_SingleOnlineUser, "Platform.Trait.SingleOnlineUser");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_CanExitApplication, "Platform.Trait.CanExitApplication");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_PlayInEditor, "Platform.Trait.PlayInEditor");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_RequiresStrictControllerMapping, "Platform.Trait.RequiresStrictControllerMapping");

UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_Input_HardwareCursor, "Platform.Trait.Input.HardwareCursor");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_Input_PrimarilyControlled, "Platform.Trait.Input.PrimarilyControlled");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_Input_PrimarilyTouchScreen, "Platform.Trait.Input.PrimarilyTouchScreen");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_Input_SupportsMouseAndKeyboard, "Platform.Trait.Input.SupportsMouseAndKeyboard");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_Input_SupportsGamepad, "Platform.Trait.Input.SupportsGamepad");
UE_DEFINE_GAMEPLAY_TAG(TAG_Platform_Trait_Input_SupportsTriggerHaptics, "Platform.Trait.Input.SupportsTriggerHaptics");


// =================================
//  ゲーム設定
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_GameSettings_Action_EditSafeZone, "GameSettings.Action.EditSafeZone");
UE_DEFINE_GAMEPLAY_TAG(TAG_GameSettings_Action_EditBrightness, "GameSettings.Action.EditBrightness");


// =================================
//  SetByCaller
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Damage, "SetByCaller.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Heal_Health, "SetByCaller.Heal.Health");
UE_DEFINE_GAMEPLAY_TAG(TAG_SetByCaller_Heal_Shield, "SetByCaller.Heal.Shield");


// =================================
//  チート
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_Cheat_UnlimitedHealth, "Cheat.UnlimitedHealth");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cheat_UnlimitedAmmo, "Cheat.UnlimitedAmmo");
UE_DEFINE_GAMEPLAY_TAG(TAG_Cheat_UnlimitedAbility, "Cheat.UnlimitedAbility");


// =================================
//  コスメティック
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_Cosmetic, "Cosmetic");


// =================================
//  状態
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_Status_IsPlayer, "Status.IsPlayer");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Spawning, "Status.Spawning");

UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Crouching, "Status.Crouching");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Running, "Status.Running");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Sprinting, "Status.Sprinting");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Aiming, "Status.Aiming");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Reloading, "Status.Reloading");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Fireing, "Status.Fireing");

UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Death, "Status.Death");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Death_Dying, "Status.Death.Dying");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Death_Dead, "Status.Death.Dead");

UE_DEFINE_GAMEPLAY_TAG(TAG_Status_AbilityInputBlocked, "Status.AbilityInputBlocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_WeaponFireBlocked, "Status.WeaponFireBlocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_WeaponAimBlocked, "Status.WeaponAimBlocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_JumpBlocked, "Status.JumpBlocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_RunBlocked, "Status.RunBlocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_SprintBlocked, "Status.SprintBlocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_MovementStopped, "Status.MovementStopped");
UE_DEFINE_GAMEPLAY_TAG(TAG_Status_DamageImmunity, "Status.DamageImmunity");


// =================================
//  初期化状態
// =================================

UE_DEFINE_GAMEPLAY_TAG(TAG_InitState_Spawned, "InitState.Spawned");
UE_DEFINE_GAMEPLAY_TAG(TAG_InitState_DataAvailable, "InitState.DataAvailable");
UE_DEFINE_GAMEPLAY_TAG(TAG_InitState_DataInitialized, "InitState.DataInitialized");
UE_DEFINE_GAMEPLAY_TAG(TAG_InitState_GameplayReady, "InitState.GameplayReady");


/// =================================
///  構造多
/// =================================

void FBEGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	GameplayTags.AddAllTags(Manager);

	// Notify manager that we are done adding native tags.
	Manager.DoneAddingNativeTags();
}

void FBEGameplayTags::AddAllTags(UGameplayTagsManager& Manager)
{
	// =================================
	//  移動状態
	// =================================

	AddMovementModeTag(Movement_Mode_Walking, "Movement.Mode.Walking", MOVE_Walking);
	AddMovementModeTag(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", MOVE_NavWalking);
	AddMovementModeTag(Movement_Mode_Falling, "Movement.Mode.Falling", MOVE_Falling);
	AddMovementModeTag(Movement_Mode_Swimming, "Movement.Mode.Swimming", MOVE_Swimming);
	AddMovementModeTag(Movement_Mode_Flying, "Movement.Mode.Flying", MOVE_Flying);
	AddMovementModeTag(Movement_Mode_Custom, "Movement.Mode.Custom", MOVE_Custom);

	AddCustomMovementModeTag(CustomMovement_Mode_Slide, "CustomMovement.Mode.Slide", 1U);
	AddCustomMovementModeTag(CustomMovement_Mode_Climb, "CustomMovement.Mode.Climb", 2U);
	AddCustomMovementModeTag(CustomMovement_Mode_WallRunR, "CustomMovement.Mode.WallRunR", 3U);
	AddCustomMovementModeTag(CustomMovement_Mode_WallRunL, "CustomMovement.Mode.WallRunL", 4U);
}

void FBEGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName), FString(TEXT("(Native) ")) + FString(TagComment));
}

void FBEGameplayTags::AddMovementModeTag(FGameplayTag& OutTag, const ANSICHAR* TagName, uint8 MovementMode)
{
	AddTag(OutTag, TagName, "Character movement mode tag.");
	GameplayTags.MovementModeTagMap.Add(MovementMode, OutTag);
}

void FBEGameplayTags::AddCustomMovementModeTag(FGameplayTag& OutTag, const ANSICHAR* TagName, uint8 CustomMovementMode)
{
	AddTag(OutTag, TagName, "Character custom movement mode tag.");
	GameplayTags.CustomMovementModeTagMap.Add(CustomMovementMode, OutTag);
}

FGameplayTag FBEGameplayTags::FindTagByString(FString TagString, bool bMatchPartialString)
{
	const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

	if (!Tag.IsValid() && bMatchPartialString)
	{
		FGameplayTagContainer AllTags;
		Manager.RequestAllGameplayTags(AllTags, true);

		for (const FGameplayTag TestTag : AllTags)
		{
			if (TestTag.ToString().Contains(TagString))
			{
				UE_LOG(LogBE, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
				Tag = TestTag;
				break;
			}
		}
	}

	return Tag;
}