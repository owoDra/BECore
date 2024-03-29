﻿// Copyright Eigi Chin

#include "BEEquipmentInstance.h"

#include "Character/Component/BEPawnBasicComponent.h"
#include "Character/BEPawnMeshAssistInterface.h"
#include "Character/BEPawnData.h"
#include "Item/Fragment/BEItemDataFragment_Equippable.h"
#include "Item/BEItemData.h"
#include "Animation/BEAnimInstance.h"
#include "BELogChannels.h"

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


UBEEquipmentInstance::UBEEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBEEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
	DOREPLIFETIME(ThisClass, ItemData);
	//DOREPLIFETIME(ThisClass, SpawnedMeshes);
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
}

void UBEEquipmentInstance::OnUnequiped()
{
}

void UBEEquipmentInstance::OnRep_ItemData()
{
	if (bDelayedActive)
	{
		bDelayedActive = false;
		OnActivated();
	}
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


void UBEEquipmentInstance::SpawnEquipmentMeshes(const TArray<FBEEquipmentMeshToSpawn>& InMeshesToSpawn)
{
	if (InMeshesToSpawn.IsEmpty())
	{
		return;
	}

	APawn* OwningPawn = GetPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogBEEquipmentSystem, Warning, TEXT("UBEEquipmentInstance::SpawnEquipmentActors: Has not valid OwningPawn"));
		return;
	}

	if (!Cast<IBEPawnMeshAssistInterface>(OwningPawn))
	{
		UE_LOG(LogBEEquipmentSystem, Warning, TEXT("UBEEquipmentInstance::SpawnEquipmentMeshes: OwningPawn Has not imple IBEPawnMeshAssistInterface"));
		return;
	}

	USkeletalMeshComponent* AttachTarget = nullptr;

	// TPP Mesh 用の Actor をスポーン
	AttachTarget = IBEPawnMeshAssistInterface::Execute_GetTPPMesh(OwningPawn);
	if (AttachTarget)
	{
		const bool bOwnerNoSee = AttachTarget->bOwnerNoSee;

		UE_LOG(LogBEEquipmentSystem, Log, TEXT("UBEEquipmentInstance::SpawnEquipmentMeshes: Create Equipment Meshes for TPP"));

		for (const FBEEquipmentMeshToSpawn& SpawnInfo : InMeshesToSpawn)
		{
			if (SpawnInfo.MeshToSpawn)
			{
				USkeletalMeshComponent* NewMesh = NewObject<USkeletalMeshComponent>(OwningPawn);
				NewMesh->SetSkeletalMesh(SpawnInfo.MeshToSpawn);
				NewMesh->SetAnimInstanceClass(SpawnInfo.MeshAnimInstance);
				NewMesh->SetRelativeTransform(SpawnInfo.AttachTransform);
				NewMesh->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);
				NewMesh->SetOwnerNoSee(bOwnerNoSee);
				NewMesh->RegisterComponent();

				SpawnedMeshes.Add(NewMesh);
			}
		}
	}

	// FPP Mesh 用の Actor をスポーン
	AttachTarget = IBEPawnMeshAssistInterface::Execute_GetFPPMesh(OwningPawn);
	if (AttachTarget)
	{
		const bool bHiddenInGame = AttachTarget->bHiddenInGame;

		UE_LOG(LogBEEquipmentSystem, Log, TEXT("UBEEquipmentInstance::SpawnEquipmentMeshes: Create Equipment Meshes for FPP"));

		for (const FBEEquipmentMeshToSpawn& SpawnInfo : InMeshesToSpawn)
		{
			if (SpawnInfo.MeshToSpawn)
			{
				USkeletalMeshComponent* NewMesh = NewObject<USkeletalMeshComponent>(OwningPawn);
				NewMesh->SetSkeletalMesh(SpawnInfo.MeshToSpawn);
				NewMesh->SetAnimInstanceClass(SpawnInfo.MeshAnimInstance);
				NewMesh->SetRelativeTransform(SpawnInfo.AttachTransform);
				NewMesh->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);
				NewMesh->SetOnlyOwnerSee(true);
				NewMesh->SetHiddenInGame(bHiddenInGame);
				NewMesh->SetCastShadow(false);
				NewMesh->RegisterComponent();

				SpawnedMeshes.Add(NewMesh);
			}
		}
	}
}

void UBEEquipmentInstance::DestroyEquipmentMeshes()
{
	for (USkeletalMeshComponent* Mesh : SpawnedMeshes)
	{
		if (Mesh)
		{
			Mesh->DestroyComponent();
		}
	}
}


void UBEEquipmentInstance::ApplyAnimLayer(TSubclassOf<UAnimInstance> InTPPLayer, TSubclassOf<UAnimInstance> InFPPLayer)
{
	APawn* OwningPawn = GetPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogBEEquipmentSystem, Warning, TEXT("UBEEquipmentInstance::ApplyAnimLayer: Has not valid OwningPawn"));
		return;
	}

	if (!Cast<IBEPawnMeshAssistInterface>(OwningPawn))
	{
		UE_LOG(LogBEEquipmentSystem, Warning, TEXT("UBEEquipmentInstance::ApplyAnimLayer: OwningPawn Has not imple IBEPawnMeshAssistInterface"));
		return;
	}

	UBEAnimInstance* AnimInstance = nullptr;

	// TPP Mesh 用の AnimLayer を適用
	if (InTPPLayer)
	{
		AnimInstance = IBEPawnMeshAssistInterface::Execute_GetTPPAnimInstance(OwningPawn);
		if (AnimInstance)
		{
			AnimInstance->LinkAnimClassLayers(InTPPLayer);
		}
	}

	// FPP Mesh 用の AnimLayer を適用
	if (InFPPLayer)
	{
		AnimInstance = IBEPawnMeshAssistInterface::Execute_GetFPPAnimInstance(OwningPawn);
		if (AnimInstance)
		{
			AnimInstance->LinkAnimClassLayers(InFPPLayer);
		}
	}
}

void UBEEquipmentInstance::RemoveAnimLayer()
{
	APawn* OwningPawn = GetPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogBEEquipmentSystem, Warning, TEXT("UBEEquipmentInstance::RemoveAnimLayer: Has not valid OwningPawn"));
		return;
	}

	if (!Cast<IBEPawnMeshAssistInterface>(OwningPawn))
	{
		UE_LOG(LogBEEquipmentSystem, Warning, TEXT("UBEEquipmentInstance::RemoveAnimLayer: OwningPawn Has not imple IBEPawnMeshAssistInterface"));
		return;
	}

	UBEAnimInstance* AnimInstance = nullptr;
	TSubclassOf<UAnimInstance> DefaultTPPAnimLayerClass = nullptr;
	TSubclassOf<UAnimInstance> DefalutFPPAnimLayerClass = nullptr;

	// PawnData から DefaultAnimLayer の設定を試行する
	if (UBEPawnBasicComponent* CharacterBasic = UBEPawnBasicComponent::FindPawnBasicComponent(OwningPawn))
	{
		if (const UBEPawnData* PawnData = CharacterBasic->GetPawnData())
		{
			DefaultTPPAnimLayerClass = PawnData->DefaultTPPAnimLayer;
			DefalutFPPAnimLayerClass = PawnData->DefaultFPPAnimLayer;
		}
	}

	// TPP Mesh 用の AnimLayer を適用
	AnimInstance = IBEPawnMeshAssistInterface::Execute_GetTPPAnimInstance(OwningPawn);
	if (AnimInstance)
	{
		AnimInstance->LinkAnimClassLayers(DefaultTPPAnimLayerClass);
	}

	// FPP Mesh 用の AnimLayer を適用
	AnimInstance = IBEPawnMeshAssistInterface::Execute_GetFPPAnimInstance(OwningPawn);
	if (AnimInstance)
	{
		AnimInstance->LinkAnimClassLayers(DefalutFPPAnimLayerClass);
	}
}


void UBEEquipmentInstance::OnActivated()
{
	if (bDelayedActive)
	{
		return;
	}

	if (!ItemData)
	{
		bDelayedActive = true;
		return;
	}

	// Fragment の情報をもとに Mesh のスポーンと AnimLayer の設定を行う
	if (const UBEItemDataFragment_Equippable* Fragment = ItemData->FindFragmentByClass<UBEItemDataFragment_Equippable>())
	{
		SpawnEquipmentMeshes(Fragment->MeshesToSpawn);
		ApplyAnimLayer(Fragment->AnimLayerToApplyToTPP, Fragment->AnimLayerToApplyToFPP);
	}

	K2_OnActivated();
}

void UBEEquipmentInstance::OnDeactivated()
{
	DestroyEquipmentMeshes();
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
