// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BEPlayerStart.h"

#include "Delegates/Delegate.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "Math/MathFwd.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Templates/SubclassOf.h"
#include "TimerManager.h"
#include "UObject/Class.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BEPlayerStart)


ABEPlayerStart::ABEPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

EBEPlayerStartLocationOccupancy ABEPlayerStart::GetLocationOccupancy(AController* const ControllerPawnToFit) const
{
	UWorld* const World = GetWorld();
	if (HasAuthority() && World)
	{
		if (AGameModeBase* AuthGameMode = World->GetAuthGameMode())
		{
			TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			FVector ActorLocation = GetActorLocation();
			const FRotator ActorRotation = GetActorRotation();

			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation, nullptr))
			{
				return EBEPlayerStartLocationOccupancy::Empty;
			}
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				return EBEPlayerStartLocationOccupancy::Partial;
			}
		}
	}

	return EBEPlayerStartLocationOccupancy::Full;
}

bool ABEPlayerStart::IsClaimed() const
{
	return ClTargetingController != nullptr;
}

bool ABEPlayerStart::TryClaim(AController* OccupyingController)
{
	if (OccupyingController != nullptr && !IsClaimed())
	{
		ClTargetingController = OccupyingController;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ExpirationTimerHandle, FTimerDelegate::CreateUObject(this, &ABEPlayerStart::CheckUnclaimed), ExpirationCheckInterval, true);
		}
		return true;
	}
	return false;
}

void ABEPlayerStart::CheckUnclaimed()
{
	if (ClTargetingController != nullptr && ClTargetingController->GetPawn() != nullptr && GetLocationOccupancy(ClTargetingController) == EBEPlayerStartLocationOccupancy::Empty)
	{
		ClTargetingController = nullptr;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
		}
	}
}
