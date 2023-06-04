// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "GameFramework/CheatManager.h"

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

#include "BEDeveloperCheatManager.generated.h"

class UBEAbilitySystemComponent;
struct FGameplayTag;

/////////////////////////////////////////////////

#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER (1 && !UE_BUILD_SHIPPING)
#endif // #ifndef USING_CHEAT_MANAGER

DECLARE_LOG_CATEGORY_EXTERN(LogBECheat, Log, All);

/////////////////////////////////////////////////

/**
 * UBEDeveloperCheatManager
 *
 *	このプロジェクトで使用される基本チートマネージャークラス。
 */
UCLASS(config = Game, Within = PlayerController, MinimalAPI)
class UBEDeveloperCheatManager : public UCheatManager
{
	GENERATED_BODY()

	//===========================================
	//  初期化
	//===========================================
public:
	UBEDeveloperCheatManager();

	virtual void InitCheatManager() override;

	//===========================================
	//  チート
	//===========================================
public:
	// Runs a cheat on the server for the owning player.
	UFUNCTION(exec)
	void Cheat(const FString& Msg);

	// Runs a cheat on the server for the all players.
	UFUNCTION(exec)
	void CheatAll(const FString& Msg);

	// Starts the next match
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void PlayNextGame();

	UFUNCTION(Exec)
	virtual void ToggleFixedCamera();

	UFUNCTION(Exec)
	virtual void CycleDebugCameras();

	UFUNCTION(Exec)
	virtual void CycleAbilitySystemDebug();

	// Forces input activated abilities to be canceled.  Useful for tracking down ability interruption bugs. 
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void CancelActivatedAbilities();

	// Adds the dynamic tag to the owning player's ability system component.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void AddTagToSelf(FString TagName);

	// Removes the dynamic tag from the owning player's ability system component.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void RemoveTagFromSelf(FString TagName);

	// Applies the specified damage amount to the owning player.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void DamageSelf(float DamageAmount);

	// Applies the specified damage amount to the actor that the player is looking at.
	virtual void DamageTarget(float DamageAmount) override;

	// Applies the specified amount of healing to the owning player.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void HealHealthSelf(float HealAmount);

	// Applies the specified amount of healing to the actor that the player is looking at.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void HealHealthTarget(float HealAmount);

	// Applies the specified amount of healing to the owning player.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void HealShieldSelf(float HealAmount);

	// Applies the specified amount of healing to the actor that the player is looking at.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void HealShieldTarget(float HealAmount);

	// Applies enough damage to kill the owning player.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void DamageSelfDestruct();

	// Prevents the owning player from taking any damage.
	virtual void God() override;

	// ダメージを受けなくなる
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void UnlimitedHealth(int32 Enabled = -1);

	// 残弾の消費がなくなる
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void UnlimitedCost(int32 Enabled = -1);


	//===========================================
	//  ユーティリティ
	//===========================================
public:
	// Helper function to write text to the console and to the log.
	static void CheatOutputText(const FString& TextToOutput);

protected:
	FGameplayTag FindTagByString(FString TagString, bool bMatchPartialString = false);

protected:
	virtual void EnableDebugCamera() override;
	virtual void DisableDebugCamera() override;
	bool InDebugCamera() const;

protected:
	virtual void EnableFixedCamera();
	virtual void DisableFixedCamera();
	bool InFixedCamera() const;

protected:
	void ApplySetByCallerDamage(UBEAbilitySystemComponent* BEASC, float DamageAmount);
	void ApplySetByCallerHealHealth(UBEAbilitySystemComponent* BEASC, float HealAmount);
	void ApplySetByCallerHealShield(UBEAbilitySystemComponent* BEASC, float HealAmount);

protected:
	UBEAbilitySystemComponent* GetPlayerAbilitySystemComponent() const;
};
