// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Cosmetics/BECosmeticAnimationTypes.h"
#include "BEWeaponActor.generated.h"

class ACharacter;
class APawn;
class AnimInstance;


/**
 * ABEWeaponActor
 *
 * 武器を装備した際にスポーンする武器のActorクラス
 * 武器の見た目に関わるデータのみ保有する
 */
UCLASS()
class ABEWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ABEWeaponActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:
	/**
	* 武器が装備されて、このActorのスポーンが完了したときに実行
	*/
	virtual void OnEquipped();

	/**
	* 武器の装備が解除されて、このActorがDestroyされようとしたときに実装
	*/
	virtual void OnUnequipped();

	/**
	* 武器がレプリケートされて、EquipmentInstanceのSpawnedActorsが同期されたときに実行
	*/
	virtual void OnNetInitialized();


protected:
	/**
	* 武器が装備されて、このActorのスポーンが完了したときに実行
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon", meta = (DisplayName = "OnEquipped"))
		void K2_OnEquipped();

	/**
	* 武器の装備が解除されて、このActorがDestroyされようとしたときに実行
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon", meta = (DisplayName = "OnUnequipped"))
		void K2_OnUnequipped();


public:
	/**
	* この武器を装備しているPawnを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		APawn* GetOwningPawn() const { return Cast<APawn>(GetOwner()); }

	/**
	* この武器を装備しているCharacterを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		ACharacter* GetOwningCharacter() const { return Cast<ACharacter>(GetOwner()); }


protected:
	/**
	* この武器を装備した時にOwning Pawnに対応するCosmetic Tagsに応じたAnim Layer
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
		FBEAnimLayerSelectionSet EquippedAnimSet;

	/**
	* EquippedAnimSetからCosmetic Tagsに応じたAnim Layerを選ぶ
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Weapon|Animation")
		TSubclassOf<UAnimInstance> PickBestAnimLayer(const FGameplayTagContainer& CosmeticTags) const;

	/**
	* Owning Pawnに対応するCosmetic Tagsを取得する
	*/
	UFUNCTION(BlueprintCallable, Category = "Weapon|Animation")
		FGameplayTagContainer GetCosmeticTagsFromOwningPawn() const;

	/**
	* Owning PawnにAnimLayerを適応する
	*/
	UFUNCTION(BlueprintCallable, Category = "Weapon|Animation")
		void ApplyAnimLayer();

	/**
	* Owning Pawnから適応したAnimLayerを削除する
	*/
	UFUNCTION(BlueprintCallable, Category = "Weapon|Animation")
		void RemoveAnimLayer();

private:
	FGameplayTagContainer OwningCosmeticTags;

	TSubclassOf<UAnimInstance> AppliedAnimLayer;

	bool HasNetInitialized = false;

	bool HasEquiped = false;
};
