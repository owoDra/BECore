// Copyright Eigi Chin

#pragma once

#include "UObject/Object.h"

#include "Containers/Array.h"
#include "System/GameplayTagStack.h"
#include "Engine/World.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BEEquipmentInstance.generated.h"

class AActor;
class UBEItemData;
class UAnimInstance;
struct FGameplayTag;


/**
 * FBEEquipmentActorToSpawn
 *
 * 装備するときのアクタースポーン設定
 */
USTRUCT(BlueprintType)
struct FBEEquipmentActorToSpawn
{
	GENERATED_BODY()

	FBEEquipmentActorToSpawn()
	{}

	UPROPERTY(EditAnywhere, Category = "Equipment")
	TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, Category = "Equipment")
	FName AttachSocket;

	UPROPERTY(EditAnywhere, Category = "Equipment")
	FTransform AttachTransform;
};


/**
 * UBEEquipmentInstance
 *
 * A piece of equipment spawned and applied to a pawn
 */
UCLASS(BlueprintType, Blueprintable)
class BECORE_API UBEEquipmentInstance : public UObject
{
	GENERATED_BODY()

public:
	UBEEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual UWorld* GetWorld() const override final;
	//~End of UObject interface

protected:
#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(
		UE::Net::FFragmentRegistrationContext& Context,
		UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS


public:
	/**
	 * OnEquiped
	 *
	 * この Equipment が EquipmentManagerComponent によって作成されたときに呼び出される。
	 * この Equipment と ItemData の関連付けと StatTag の初期化を行う
	 */
	virtual void OnEquiped(const UBEItemData* InItemData);

	/**
	 * OnUnequiped
	 *
	 * この Equipment が EquipmentManagerComponent によって取り除かれるときに呼び出される。
	 * この SpawnedActor の除去を行う。
	 */
	virtual void OnUnequiped();

private:
	// この Equipment の ItemData
	UPROPERTY(Replicated)
	TObjectPtr<const UBEItemData> ItemData;


public:
	/**
	 * AddStatTagStack
	 *
	 * Equipment に統計情報として扱える Tag を追加する (StackCount が 0 以下の場合は何もしない)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	/**
	 * RemoveStatTagStack
	 *
	 * Equipment に統計情報として扱える Tag を削除する (StackCount が 0 以下の場合は何もしない)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	/**
	 * GetStatTagStackCount
	 *
	 * Equipment に統計情報として扱える Tag がいくつあるかを返す (存在しない場合は 0 を返す)
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	/**
	 * HasStatTag
	 *
	 * Equipment に統計情報として扱える Tag が存在するかどうかを返す
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool HasStatTag(FGameplayTag Tag) const;

	// この Equipment の初期 StatTag
	// Equipment が作成された際の初期化処理に使用する
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TMap<FGameplayTag, int32> InitialEquipmentStats;

private:
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;


public:
	/**
	 * SpawnEquipmentActors
	 *
	 * Equipment が Active になったときに呼び出される。
	 * Equipment の見た目となる Actor をスポーンする。
	 * ActorToSpawn がからの場合は何もスポーンしない。
	 */
	virtual void SpawnEquipmentActors();

	/**
	 * DestroyEquipmentActors
	 *
	 * Equipment が Deactive になったときに呼び出される。
	 * Equipment の見た目となる Actor を破棄する。
	 */
	virtual void DestroyEquipmentActors();

	// Equipment が Active になったときにスポーンする
	// 装備品の見た目となる Actor のクラス
	// 設定しない場合は何もスポーンしない。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TArray<FBEEquipmentActorToSpawn> ActorsToSpawn;

private:
	UPROPERTY(Replicated) 
	TArray<TObjectPtr<AActor>> SpawnedActors;


public:
	/**
	 * ApplyAnimLayer
	 * 
	 * Equipment が Active になったときに呼び出される。
	 * Pawn に AnimLayer を適応する。
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void ApplyAnimLayer();

	/**
	 * RemoveAnimLayer
	 * 
	 * Equipment が Deactive になったときに呼び出される。
	 * Pawn から適応した AnimLayer を削除する。
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void RemoveAnimLayer();

	// Equipment が Active になったときに Pawn に
	// 適応する AnimLayer のクラス
	// 設定しない場合は何も適応されない。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<UAnimInstance> AnimLayerToApply;


public:
	/**
	 * OnActivated
	 *
	 * EquipmentManagerComponent からこの Equipment を Active にしたときに呼び出される
	 */
	virtual void OnActivated();

	/**
	 * OnDeactivated
	 *
	 * EquipmentManagerComponent からこの Equipment を Deactive にしたときに呼び出される
	 */
	virtual void OnDeactivated();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment", meta = (DisplayName = "OnActivated"))
	void K2_OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment", meta = (DisplayName = "OnDeactivated"))
	void K2_OnDeactivated();


public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	APawn* GetPawn() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment", meta = (DeterminesOutputType = PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	const UBEItemData* GetItemData() const { return ItemData; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	TArray<AActor*> GetSpawnedActors() const { return SpawnedActors; }
};
