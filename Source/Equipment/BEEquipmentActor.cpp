// Copyright Eigi Chin

#include "Weapons/BEWeaponActor.h"
#include "BEGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Cosmetics/BEPawnComponent_CharacterParts.h"


ABEWeaponActor::ABEWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ABEWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	OnEquipped();
}

void ABEWeaponActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnUnequipped();
}

void ABEWeaponActor::OnEquipped()
{
	if (!HasNetInitialized || HasEquiped)
	{
		return;
	}

	HasEquiped = true;

	ApplyAnimLayer();

	K2_OnEquipped();
}

void ABEWeaponActor::OnUnequipped()
{
	RemoveAnimLayer();

	K2_OnUnequipped();
}

void ABEWeaponActor::OnNetInitialized()
{
	HasNetInitialized = true;

	OnEquipped();
}

TSubclassOf<UAnimInstance> ABEWeaponActor::PickBestAnimLayer(const FGameplayTagContainer& CosmeticTags) const
{
	return EquippedAnimSet.SelectBestLayer(CosmeticTags);
}

FGameplayTagContainer ABEWeaponActor::GetCosmeticTagsFromOwningPawn() const
{
	if (!GetOwner())
	{
		return FGameplayTagContainer();
	}
	
	if (UBEPawnComponent_CharacterParts* Parts = Cast<UBEPawnComponent_CharacterParts>(
		GetOwner()->GetComponentByClass(UBEPawnComponent_CharacterParts::StaticClass())))
	{
		return Parts->GetCombinedTags(TAG_Cosmetic);
	}

	return FGameplayTagContainer();
}

void ABEWeaponActor::ApplyAnimLayer()
{
	OwningCosmeticTags = GetCosmeticTagsFromOwningPawn();

	AppliedAnimLayer = PickBestAnimLayer(OwningCosmeticTags);

	if (AppliedAnimLayer)
	{
		if (ACharacter* Chara = GetOwningCharacter())
		{
			if (USkeletalMeshComponent* Mesh = Chara->GetMesh())
			{
				Mesh->LinkAnimClassLayers(AppliedAnimLayer);
			}
		}
	}
}

void ABEWeaponActor::RemoveAnimLayer()
{
	if (AppliedAnimLayer)
	{
		if (ACharacter* Chara = GetOwningCharacter())
		{
			if (USkeletalMeshComponent* Mesh = Chara->GetMesh())
			{
				Mesh->UnlinkAnimClassLayers(AppliedAnimLayer);
			}
		}
	}
}
