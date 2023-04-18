// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEEquipmentInstance.h"

#include "BEEquipmentDefinition.h"
#include "Weapons/BEWeaponActor.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Math/Transform.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "Templates/Casts.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEEquipmentInstance)

class FLifetimeProperty;
class UClass;
class USceneComponent;


UBEEquipmentInstance::UBEEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UWorld* UBEEquipmentInstance::GetWorld() const
{
	if (APawn* OwningPawn = GetPawn())
	{
		return OwningPawn->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

void UBEEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}

#if UE_WITH_IRIS
void UBEEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	Super::RegisterReplicationFragments(Context, RegistrationFlags);

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

APawn* UBEEquipmentInstance::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}

APawn* UBEEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (GetOuter()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}
	return Result;
}

void UBEEquipmentInstance::SpawnEquipmentActors(const TArray<FBEEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}

		for (const FBEEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			ABEWeaponActor* NewActor = GetWorld()->SpawnActorDeferred<ABEWeaponActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
			NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
			NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

			SpawnedActors.Add(NewActor);

			OnRep_SpawnedActors();
		}
	}
}

void UBEEquipmentInstance::DestroyEquipmentActors()
{
	for (ABEWeaponActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}

	SpawnedActors.Empty();
	OnRep_SpawnedActors();
}

void UBEEquipmentInstance::OnEquipped()
{
	K2_OnEquipped();
}

void UBEEquipmentInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

void UBEEquipmentInstance::OnRep_Instigator()
{
}

void UBEEquipmentInstance::OnRep_SpawnedActors()
{
	for (ABEWeaponActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->OnNetInitialized();
		}
	}
}
