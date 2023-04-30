// Copyright Eigi Chin

#include "BECameraMode_FirstPerson.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "Character/BECharacter.h"
#include "BECharacterCameraComponent.h"
#include "Components/CapsuleComponent.h"


// =================================
//  初期化
// =================================

UBECameraMode_FirstPerson::UBECameraMode_FirstPerson()
{
	FPPMaskParamName = FName(TEXT("FPP Mask"));
	FPPMaterialIndexes.Empty();
	FPPMaterialIndexes.Add(1);
}

void UBECameraMode_FirstPerson::OnActivation()
{
	Super::OnActivation();

	if (ACharacter* Target = Cast<ACharacter>(GetTargetActor()))
	{
		if (Target->IsLocallyControlled())
		{
			if (ABECharacter* BEChara = Cast<ABECharacter>(Target))
			{
				BEChara->UpdateMeshPositionWhenCrouch = false;
			}

			Target->ForEachComponent(true, [=](UActorComponent* InComponent)
			{
				if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(InComponent))
				{
					MeshComponent->SetScalarParameterValueOnMaterials(FPPMaskParamName, 1.f);
					MeshComponent->SetCastShadow(false);

					if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(MeshComponent))
					{
						int NumMat = SkeletalMeshComponent->GetNumMaterials();
						for (int i = 0; i < NumMat; i++)
						{
							if (!FPPMaterialIndexes.Contains(i))
							{
								SkeletalMeshComponent->ShowMaterialSection(i, 0, false, 0);
							}
						}
					}
				}
			});

			TArray<AActor*> OutActors;
			Target->GetAttachedActors(OutActors);
			for (AActor* AttachedActor : OutActors)
			{
				AttachedActor->ForEachComponent(true, [=](UActorComponent* InComponent)
				{
					if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(InComponent))
					{
						MeshComponent->SetScalarParameterValueOnMaterials(FPPMaskParamName, 1.f);
						MeshComponent->SetCastShadow(false);
					}
				});
			}

			Target->GetMesh()->AttachToComponent(GetBECharacterCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
}

void UBECameraMode_FirstPerson::OnDeactivation()
{
	Super::OnDeactivation();

	if (ACharacter* Target = Cast<ACharacter>(GetTargetActor()))
	{
		if (Target->IsLocallyControlled())
		{
			if (ABECharacter* BEChara = Cast<ABECharacter>(Target))
			{
				BEChara->UpdateMeshPositionWhenCrouch = true;
			}

			Target->GetMesh()->AttachToComponent(Target->GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);

			Target->ForEachComponent(true, [=](UActorComponent* InComponent)
			{
				if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(InComponent))
				{
					MeshComponent->SetScalarParameterValueOnMaterials(FPPMaskParamName, 0.f);
					MeshComponent->SetCastShadow(true);

					if (USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(MeshComponent))
					{
						int NumMat = SkeletalMeshComponent->GetNumMaterials();
						for (int i = 0; i < NumMat; i++)
						{
							if (!FPPMaterialIndexes.Contains(i))
							{
								SkeletalMeshComponent->ShowMaterialSection(i, 0, true, 0);
							}
						}
					}
				}
			});

			TArray<AActor*> OutActors;
			Target->GetAttachedActors(OutActors);
			for (AActor* AttachedActor : OutActors)
			{
				AttachedActor->ForEachComponent(true, [=](UActorComponent* InComponent)
				{
					if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(InComponent))
					{
						MeshComponent->SetScalarParameterValueOnMaterials(FPPMaskParamName, 0.f);
						MeshComponent->SetCastShadow(true);
					}
				});
			}
		}
	}
}


// =================================
//  カメラ
// =================================

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
