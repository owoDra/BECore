// Copyright Eigi Chin

#include "BEEquipmentInstance.h"

#include "Character/BEPawnMeshAssistInterface.h"
#include "Animation/BEAnimInstance.h"

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

void UBEEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
	DOREPLIFETIME(ThisClass, ItemData);
	DOREPLIFETIME(ThisClass, SpawnedActors);
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

#if UE_WITH_IRIS
void UBEEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	Super::RegisterReplicationFragments(Context, RegistrationFlags);

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS


void UBEEquipmentInstance::OnEquiped(const UBEItemData* InItemData)
{
	ItemData = InItemData;

	for (const auto& KVP : InitialEquipmentStats)
	{
		AddStatTagStack(KVP.Key, KVP.Value);
	}
}

void UBEEquipmentInstance::OnUnequiped()
{
}


void UBEEquipmentInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void UBEEquipmentInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 UBEEquipmentInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool UBEEquipmentInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}


void UBEEquipmentInstance::SpawnEquipmentActors()
{
	if (ActorsToSpawn.IsEmpty())
	{
		return;
	}

	if (APawn* OwningPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}

		for (const FBEEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			if (SpawnInfo.ActorToSpawn)
			{
				AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
				NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
				NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
				NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

				SpawnedActors.Add(NewActor);
			}
		}
	}
}

void UBEEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}


void UBEEquipmentInstance::ApplyAnimLayer()
{
	if (!AnimLayerToApply)
	{
		return;
	}

	if (APawn* OwningPawn = GetPawn())
	{
		if (Cast<IBEPawnMeshAssistInterface>(OwningPawn))
		{
			TArray<UBEAnimInstance*> AnimInstances;
			IBEPawnMeshAssistInterface::Execute_GetMainAnimInstances(OwningPawn, AnimInstances);
			for (UBEAnimInstance* AnimIns : AnimInstances)
			{
				AnimIns->LinkAnimClassLayers(AnimLayerToApply);
			}
		}
	}
}

void UBEEquipmentInstance::RemoveAnimLayer()
{
	if (!AnimLayerToApply)
	{
		return;
	}

	if (APawn* OwningPawn = GetPawn())
	{
		if (Cast<IBEPawnMeshAssistInterface>(OwningPawn))
		{
			TArray<UBEAnimInstance*> AnimInstances;
			IBEPawnMeshAssistInterface::Execute_GetMainAnimInstances(OwningPawn, AnimInstances);
			for (UBEAnimInstance* AnimIns : AnimInstances)
			{
				AnimIns->UnlinkAnimClassLayers(AnimLayerToApply);
			}
		}
	}
}


void UBEEquipmentInstance::OnActivated()
{
	SpawnEquipmentActors();
	ApplyAnimLayer();
	K2_OnActivated();
}

void UBEEquipmentInstance::OnDeactivated()
{
	DestroyEquipmentActors();
	RemoveAnimLayer();
	K2_OnDeactivated();
}


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
