// Copyright Eigi Chin

#pragma once

#include "UObject/Object.h"

#include "Containers/Array.h"
#include "System/GameplayTagStack.h"
#include "Engine/World.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BEEquipmentInstance.generated.h"

class UBEItemData;
class UAnimInstance;
class USkeletalMesh;
class USkeletalMeshComponent;
struct FGameplayTag;


/**
 * FBEEquipmentMeshToSpawn
 *
 * 装備するときのアクタースポーン設定
 */
USTRUCT(BlueprintType)
struct FBEEquipmentMeshToSpawn
{
	GENERATED_BODY()

public:
	FBEEquipmentMeshToSpawn() {}

	UPROPERTY(EditAnywhere, Category = "Equipment")
	TObjectPtr<USkeletalMesh> MeshToSpawn;

	UPROPERTY(EditAnywhere, Category = "Equipment")
	TSubclassOf<UAnimInstance> MeshAnimInstance;

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

protected:
	/**
	 * OnRep_ItemData
	 *
	 * ItemData が同期されたときに実行される
	 */
	UFUNCTION()
	virtual void OnRep_ItemData();

private:
	// この Equipment の ItemData
	UPROPERTY(ReplicatedUsing = "OnRep_ItemData")
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

private:
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;


public:
	/**
	 * SpawnEquipmentMeshes
	 *
	 * Equipment が Active になったときに呼び出される。
	 * Equipment の見た目となる SkeletalMeshComponent をスポーンする。
	 * ActorToSpawn がからの場合は何もスポーンしない。
	 */
	virtual void SpawnEquipmentMeshes(const TArray<FBEEquipmentMeshToSpawn>& InMeshesToSpawn);

	/**
	 * DestroyEquipmentMeshes
	 *
	 * Equipment が Deactive になったときに呼び出される。
	 * Equipment の見た目となる SkeletalMeshComponent を破棄する。
	 */
	virtual void DestroyEquipmentMeshes();

private:
	//UPROPERTY(Replicated) 
	TArray<TObjectPtr<USkeletalMeshComponent>> SpawnedMeshes;


public:
	/**
	 * ApplyAnimLayer
	 * 
	 * Equipment が Active になったときに呼び出される。
	 * Pawn に AnimLayer を適応する。
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void ApplyAnimLayer(TSubclassOf<UAnimInstance> InTPPLayer, TSubclassOf<UAnimInstance> InFPPLayer);

	/**
	 * RemoveAnimLayer
	 * 
	 * Equipment が Deactive になったときに呼び出される。
	 * Pawn から適応した AnimLayer を削除する。
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void RemoveAnimLayer();


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

private:
	// ItemData がまだ同期されていない状態で OnActivated が呼び出されたときに
	// bDelayedActive を True に設定し、同期が完了したときに再度 OnActivated を
	// 呼び出すように要求するためのフラグ。
	bool bDelayedActive = false;


public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	APawn* GetPawn() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment", meta = (DeterminesOutputType = PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	const UBEItemData* GetItemData() const { return ItemData; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
	TArray<USkeletalMeshComponent*> GetSpawnedMeshes() const { return SpawnedMeshes; }
};
