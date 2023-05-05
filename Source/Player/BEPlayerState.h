// Copyright Eigi Chin

#pragma once

#include "ModularPlayerState.h"

#include "Message/BEVerbMessage.h"
#include "Team/BETeamAgentInterface.h"
#include "Ability/BEAbilitySet.h"

#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "HAL/Platform.h"
#include "System/GameplayTagStack.h"
#include "Templates/Casts.h"
#include "UObject/UObjectGlobals.h"

#include "BEPlayerState.generated.h"

class UBEPawnData;
class ABEPlayerController;
class UBEExperienceDefinition;
class UBEAbilitySystemComponent;
class UAbilitySystemComponent;
class AController;
class APlayerState;
class FName;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FBEAbilitySet_GrantedHandles;


/**
 * EBEPlayerConnectionType
 *
 *	Client がゲームにどの役割で接続(参加)しているかを示す
 */
UENUM()
enum class EBEPlayerConnectionType : uint8
{
	// 実際にゲームに参加して活動しているプレイヤー
	Player = 0,

	// リアルタイムなゲームに接続しているが参加はしていない観戦者
	LiveSpectator,

	// 記録したゲームをリプレイしている観戦者
	ReplaySpectator,

	// 既にゲームから切断されているプレイヤー
	InactivePlayer
};


/**
 * ABEPlayerState
 *
 *	このプロジェクトで使用されるベースの PlayerState クラス。
 */
UCLASS(Config = Game)
class BECORE_API ABEPlayerState 
	: public AModularPlayerState
	, public IAbilitySystemInterface
	, public IBETeamAgentInterface
{
	GENERATED_BODY()

public:
	ABEPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const FName NAME_BEAbilityReady;

public:
	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor interface

	//~APlayerState interface
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnDeactivated() override;
	virtual void OnReactivated() override;
	//~End of APlayerState interface

private:
	void OnExperienceLoaded(const UBEExperienceDefinition* CurrentExperience);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	FBEAbilitySet_GrantedHandles PawnDataAbilityHandles;


private:
	// Player の Pawn または Character のスポーンに用いる 基本情報。
	// GameMode はこの PawnData の情報をもとに Player の Pawn または Character をスポーンさせる
	UPROPERTY(Replicated)
	TObjectPtr<const UBEPawnData> PawnData;

public:
	/**
	 * SetPawnData
	 *
	 *	Player の Pawn または Character のスポーンに用いるPawnData を設定する。
	 *  実行にはサーバー権限が必要。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PlayerState", meta = (DisplayName = "SetPawnData"))
	void K2_SetPawnData(const UBEPawnData* InPawnData);
	void SetPawnData(const UBEPawnData* InPawnData);

	/**
	 * GetPawnData
	 *
	 *	Player の Pawn または Character のスポーンに用いるPawnData を返す
	 */
	const UBEPawnData* GetPawnData() const { return PawnData; }

	
private:
	// Player のゲームへの接続(参加)状態
	UPROPERTY(Replicated)
	EBEPlayerConnectionType MyPlayerConnectionType;

public:
	/**
	 * SetPlayerConnectionType
	 *
	 *	Player のゲームへの接続(参加)状態を設定する
	 */
	void SetPlayerConnectionType(EBEPlayerConnectionType NewType);

	/**
	 * GetPlayerConnectionType
	 *
	 *	Player のゲームへの接続(参加)状態を返す
	 */
	EBEPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }
	

private:
	// Player が属する Team が変更された時に知らせるためのデリゲート
	UPROPERTY()
	FOnBETeamIndexChangedDelegate OnTeamChangedDelegate;

	// PLayer が属する Team の ID
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	// Player が属する Team 内での グループ分け用の ID
	// 基本的には大人数での対戦ゲームなどで使用する。
	UPROPERTY(ReplicatedUsing = OnRep_MySquadID)
	int32 MySquadID;

	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);

	UFUNCTION()
	void OnRep_MySquadID();

public:
	//~IBETeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnBETeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IBETeamAgentInterface interface

	/**
	 * GetTeamId
	 *
	 *	Player の属する Team の ID を整数値として返す
	 */
	UFUNCTION(BlueprintCallable)
	int32 GetTeamId() const { return GenericTeamIdToInteger(MyTeamID); }

	/**
	 * SetSquadID
	 *
	 *	Player が属する Team 内での グループ分け用の ID を設定する
	 */
	void SetSquadID(int32 NewSquadID);

	/**
	 * SetSquadID
	 *
	 *	Player が属する Team 内での グループ分け用の ID を返す
	 */
	UFUNCTION(BlueprintCallable)
	int32 GetSquadId() const { return MySquadID; }


private:
	// Player が保有する 統計情報としてカウント可能な Tag の情報
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

protected:
	const FGameplayTagStackContainer& GetStatTags() const { return StatTags; }
	void SetStatTags(const FGameplayTagStackContainer& InStatTags) { StatTags = InStatTags; }

public:
	/**
	 * AddStatTagStack
	 *
	 * Player に統計情報として扱える Tag を追加する (StackCount が 0 以下の場合は何もしない)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	/**
	 * RemoveStatTagStack
	 *
	 * Player に統計情報として扱える Tag を削除する (StackCount が 0 以下の場合は何もしない)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	/**
	 * GetStatTagStackCount
	 *
	 * Player に統計情報として扱える Tag がいくつあるかを返す (存在しない場合は 0 を返す)
	 */
	UFUNCTION(BlueprintCallable, Category=Teams)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	/**
	 * HasStatTag
	 *
	 * Player に統計情報として扱える Tag が存在するかどうかを返す
	 */
	UFUNCTION(BlueprintCallable, Category=Teams)
	bool HasStatTag(FGameplayTag Tag) const;

	
public:
	/**
	 * ClientBroadcastMessage
	 *
	 * この Player にのみメッセージを送信する。
	 * 他のプレイヤーからの称賛などといった、必ずしも重要ではない処理にのみ使用
	 */
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "PlayerState")
	void ClientBroadcastMessage(const FBEVerbMessage Message);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	ABEPlayerController* GetBEPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};
