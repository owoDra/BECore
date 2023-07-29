// Copyright owoDra

#include "BECameraMode_FirstPerson.h"

#include "Character/BEPawnMeshAssistInterface.h"
#include "Character/BECharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Templates/Casts.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECameraMode_FirstPerson)

class USceneComponent;


void UBECameraMode_FirstPerson::PostActivateMode()
{
	if (ABECharacter* BEChara = Cast<ABECharacter>(GetTargetActor()))
	{
		if (BEChara->IsLocallyControlled())
		{
			if (Cast<IBEPawnMeshAssistInterface>(BEChara))
			{
				TArray<USkeletalMeshComponent*> Meshes;

				// FPP Mesh の表示
				IBEPawnMeshAssistInterface::Execute_GetFPPMeshes(BEChara, Meshes);
				for (USkeletalMeshComponent* Mesh :Meshes)
				{
					Mesh->SetHiddenInGame(false, true);
				}

				// TPP Mesh の非表示
				IBEPawnMeshAssistInterface::Execute_GetTPPMeshes(BEChara, Meshes);
				for (USkeletalMeshComponent* Mesh : Meshes)
				{
					Mesh->SetOwnerNoSee(true);
				}
			}

			// しゃがみ時の Mesh 位置の更新を無効にする
			//BEChara->UpdateMeshPositionWhenCrouch = false;
		}
	}
}

void UBECameraMode_FirstPerson::PreDeactivateMode()
{
	if (ABECharacter* BEChara = Cast<ABECharacter>(GetTargetActor()))
	{
		if (BEChara->IsLocallyControlled())
		{
			if (Cast<IBEPawnMeshAssistInterface>(BEChara))
			{
				TArray<USkeletalMeshComponent*> Meshes;

				// FPP Mesh の表示
				IBEPawnMeshAssistInterface::Execute_GetFPPMeshes(BEChara, Meshes);
				for (USkeletalMeshComponent* Mesh : Meshes)
				{
					Mesh->SetHiddenInGame(true, true);
				}

				// TPP Mesh の非表示
				IBEPawnMeshAssistInterface::Execute_GetTPPMeshes(BEChara, Meshes);
				for (USkeletalMeshComponent* Mesh : Meshes)
				{
					Mesh->SetOwnerNoSee(false);
				}
			}

			// しゃがみ時の Mesh 位置の更新を有効にする
			//BEChara->UpdateMeshPositionWhenCrouch = true;
		}
	}
}


void UBECameraMode_FirstPerson::UpdateView(float DeltaTime)
{
	UpdateForTarget(DeltaTime);
	UpdateCrouchOffset(DeltaTime);

	FVector PivotLocation = GetPivotLocation() + CurrentCrouchOffset;
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void UBECameraMode_FirstPerson::UpdateForTarget(float DeltaTime)
{
	if (const ACharacter* TargetCharacter = Cast<ACharacter>(GetTargetActor()))
	{
		if (TargetCharacter->bIsCrouched)
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			const float CrouchedHeightAdjustment = TargetCharacterCDO->CrouchedEyeHeight - TargetCharacterCDO->BaseEyeHeight;

			SetTargetCrouchOffset(FVector(0.f, 0.f, CrouchedHeightAdjustment));

			return;
		}
	}

	SetTargetCrouchOffset(FVector::ZeroVector);
}

void UBECameraMode_FirstPerson::SetTargetCrouchOffset(FVector NewTargetOffset)
{
	CrouchOffsetBlendPct = 0.0f;
	InitialCrouchOffset = CurrentCrouchOffset;
	TargetCrouchOffset = NewTargetOffset;
}

void UBECameraMode_FirstPerson::UpdateCrouchOffset(float DeltaTime)
{
	if (CrouchOffsetBlendPct < 1.0f)
	{
		CrouchOffsetBlendPct = FMath::Min(CrouchOffsetBlendPct + DeltaTime * CrouchOffsetBlendMultiplier, 1.0f);
		CurrentCrouchOffset = FMath::InterpEaseInOut(InitialCrouchOffset, TargetCrouchOffset, CrouchOffsetBlendPct, 1.0f);
	}
	else
	{
		CurrentCrouchOffset = TargetCrouchOffset;
		CrouchOffsetBlendPct = 1.0f;
	}
}
