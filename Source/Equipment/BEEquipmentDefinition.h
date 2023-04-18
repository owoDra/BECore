// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "UObject/Object.h"

#include "Containers/Array.h"
#include "Math/Transform.h"
#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectGlobals.h"

#include "BEEquipmentDefinition.generated.h"

class ABEWeaponActor;
class UBEAbilitySet;
class UBEEquipmentInstance;

USTRUCT()
struct FBEEquipmentActorToSpawn
{
	GENERATED_BODY()

	FBEEquipmentActorToSpawn()
	{}

	UPROPERTY(EditAnywhere, Category=Equipment)
	TSubclassOf<ABEWeaponActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


/**
 * UBEEquipmentDefinition
 *
 * Definition of a piece of equipment that can be applied to a pawn
 */
UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class UBEEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	UBEEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// クイックスロットに追加されたときにそのまま装備するかどうか
	// タクティカルアビリティなどでGameplay Abilityを登録と同時に付与する場合にTrueにする
	UPROPERTY(EditDefaultsOnly, Category = Equipment)
		bool EquipWhenAdded;

	// 装備品の性能や状態を管理するためのクラス
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
		TSubclassOf<UBEEquipmentInstance> InstanceType;

	// 装備時に装備したPawnに付与するアビリティセット
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
		TArray<TObjectPtr<const UBEAbilitySet>> AbilitySetsToGrant;

	// 装備品の見た目を担うActorのクラス
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
		TArray<FBEEquipmentActorToSpawn> ActorsToSpawn;
};
