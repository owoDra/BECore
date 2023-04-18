// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEUICameraManagerComponent.h"

#include "BEPlayerCameraManager.h"

#include "Delegates/Delegate.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CoreMisc.h"
#include "Templates/Casts.h"
#include "Templates/UnrealTemplate.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEUICameraManagerComponent)


UBEUICameraManagerComponent* UBEUICameraManagerComponent::GetComponent(APlayerController* PC)
{
	if (PC != nullptr)
	{
		if (ABEPlayerCameraManager* PCCamera = Cast<ABEPlayerCameraManager>(PC->PlayerCameraManager))
		{
			return PCCamera->GetUICameraComponent();
		}
	}

	return nullptr;
}

UBEUICameraManagerComponent::UBEUICameraManagerComponent()
{
	bWantsInitializeComponent = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// Register "showdebug" hook.
		if (!IsRunningDedicatedServer())
		{
			AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);
		}
	}
}

void UBEUICameraManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UBEUICameraManagerComponent::SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams)
{
	TGuardValue<bool> UpdatingViewTargetGuard(bUpdatingViewTarget, true);

	ViewTarget = InViewTarget;
	CastChecked<ABEPlayerCameraManager>(GetOwner())->SetViewTarget(ViewTarget, TransitionParams);
}

bool UBEUICameraManagerComponent::NeedsToUpdateViewTarget() const
{
	return false;
}

void UBEUICameraManagerComponent::UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime)
{
}

void UBEUICameraManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
}