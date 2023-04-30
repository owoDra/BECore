// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"

#include "BEBotCreationComponent.generated.h"

class UBEExperienceDefinition;
class UBECharacterData;
class AAIController;


UCLASS(Blueprintable, Abstract)
class UBEBotCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UBEBotCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void BeginPlay() override;
	//~End of UActorComponent interface

private:
	void OnExperienceLoaded(const UBEExperienceDefinition* Experience);

protected:
	UPROPERTY(EditDefaultsOnly, Category=Teams)
	int32 NumBotsToCreate = 5;

	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TSubclassOf<AAIController> BotControllerClass;

	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<FString> RandomBotNames;

	TArray<FString> RemainingBotNames;

protected:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AAIController>> SpawnedBotList;

#if WITH_SERVER_CODE
public:
	void Cheat_AddBot() { SpawnOneBot(); }
	void Cheat_RemoveBot() { RemoveOneBot(); }

protected:
	virtual void ServerCreateBots();

	virtual void SpawnOneBot();
	virtual void RemoveOneBot();

	FString CreateBotName(int32 PlayerIndex);
#endif
};
