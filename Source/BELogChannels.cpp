// Copyright owoDra

#include "BELogChannels.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogBE);
DEFINE_LOG_CATEGORY(LogBEExperience);
DEFINE_LOG_CATEGORY(LogBEMovement);
DEFINE_LOG_CATEGORY(LogBEAbilitySystem);
DEFINE_LOG_CATEGORY(LogBEEquipmentSystem);
DEFINE_LOG_CATEGORY(LogBEInventorySystem);
DEFINE_LOG_CATEGORY(LogBEWeaponSystem);
DEFINE_LOG_CATEGORY(LogBETeams);

FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}
