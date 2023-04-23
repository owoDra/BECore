// Copyright Eigi Chin

#pragma once

#include "UObject/Object.h"

#include "Containers/Array.h"
#include "Engine/World.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"

#include "BEEquipmentInstance.generated.h"

class AActor;
class UBEItemData;
struct FGameplayTag;


/**
 * FBEEquipmentActorToSpawn
 *
 * ��������Ƃ��̃A�N�^�[�X�|�[���ݒ�
 */
USTRUCT()
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
class UBEEquipmentInstance : public UObject
{
	GENERATED_BODY()

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
	 * ���� Equipment �� EquipmentManagerComponent �ɂ���č쐬���ꂽ�Ƃ��ɌĂяo�����B
	 * ���� Equipment �� ItemData �̊֘A�t���� StatTag �̏��������s��
	 */
	virtual void OnEquiped(UBEItemData* InItemData);

	/**
	 * OnUnequiped
	 *
	 * ���� Equipment �� EquipmentManagerComponent �ɂ���Ď�菜�����Ƃ��ɌĂяo�����B
	 * ���� SpawnedActor �̏������s���B
	 */
	virtual void OnUnequiped();

	// ���� Equipment �� ItemData
	UPROPERTY(Replicated)
	TObjectPtr<UBEItemData> ItemData;


public:
	/**
	 * AddStatTagStack
	 *
	 * Equipment �ɓ��v���Ƃ��Ĉ����� Tag ��ǉ����� (StackCount �� 0 �ȉ��̏ꍇ�͉������Ȃ�)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	/**
	 * RemoveStatTagStack
	 *
	 * Equipment �ɓ��v���Ƃ��Ĉ����� Tag ���폜���� (StackCount �� 0 �ȉ��̏ꍇ�͉������Ȃ�)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	/**
	 * GetStatTagStackCount
	 *
	 * Equipment �ɓ��v���Ƃ��Ĉ����� Tag ���������邩��Ԃ� (���݂��Ȃ��ꍇ�� 0 ��Ԃ�)
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	/**
	 * HasStatTag
	 *
	 * Equipment �ɓ��v���Ƃ��Ĉ����� Tag �����݂��邩�ǂ�����Ԃ�
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool HasStatTag(FGameplayTag Tag) const;

	// ���� Equipment �̏��� StatTag
	// Equipment ���쐬���ꂽ�ۂ̏����������Ɏg�p����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TMap<FGameplayTag, int32> InitialEquipmentStats;


public:
	/**
	 * OnActivated
	 *
	 * EquipmentManagerComponent ���炱�� Equipment �� Active �ɂ����Ƃ��ɌĂяo�����
	 */
	virtual void OnActivated();

	/**
	 * OnDeactivated
	 *
	 * EquipmentManagerComponent ���炱�� Equipment �� Deactive �ɂ����Ƃ��ɌĂяo�����
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


//public:
//	/**
//	 * SpawnEquipmentActors
//	 *
//	 * �����̌����ڂ�S�� Actor ���X�|�[������
//	 */
//	UFUNCTION(BlueprintCallable, Category = "Equipment")
//	virtual void SpawnEquipmentActors(const TArray<FBEEquipmentActorToSpawn>& ActorsToSpawn);
//
//	/**
//	 * DestroyEquipmentActors
//	 *
//	 * SpawnedActor ����������
//	 */
//	UFUNCTION(BlueprintCallable, Category = "Equipment")
//	virtual void DestroyEquipmentActors();
//
//private:
//	UFUNCTION()
//	void OnRep_SpawnedActors();
//
//	UPROPERTY(ReplicatedUsing = OnRep_SpawnedActors)
//	TArray<TObjectPtr<AActor>> SpawnedActors;
//
//
//public:
//
//	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
//	TArray<AActor*> GetSpawnedActors() const { return SpawnedActors; }
};
