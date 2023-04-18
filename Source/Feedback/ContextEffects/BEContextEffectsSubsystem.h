// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "BEContextEffectsSubsystem.generated.h"

class UBEContextEffectsLibrary;
class UNiagaraComponent;

/**
 *
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "BEContextEffects"))
class BECORE_API UBEContextEffectsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	//
	UPROPERTY(config, EditAnywhere)
	TMap<TEnumAsByte<EPhysicalSurface>, FGameplayTag> SurfaceTypeToContextMap;
};

/**
 *
 */
UCLASS()
class BECORE_API UBEContextEffectsSet : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TSet<UBEContextEffectsLibrary*> BEContextEffectsLibraries;
};


/**
 * 
 */
UCLASS()
class BECORE_API UBEContextEffectsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	/** */
	UFUNCTION(BlueprintCallable, Category = "ContextEffects")
	void SpawnContextEffects(
		const AActor* SpawningActor
		, USceneComponent* AttachToComponent
		, const FName AttachPoint
		, const FVector LocationOffset
		, const FRotator RotationOffset
		, FGameplayTag Effect
		, FGameplayTagContainer Contexts
		, TArray<UAudioComponent*>& AudioOut
		, TArray<UNiagaraComponent*>& NiagaraOut
		, FVector VFXScale = FVector(1)
		, float AudioVolume = 1
		, float AudioPitch = 1);

	/** */
	UFUNCTION(BlueprintCallable, Category = "ContextEffects")
	bool GetContextFromSurfaceType(TEnumAsByte<EPhysicalSurface> PhysicalSurface, FGameplayTag& Context);

	/** */
	UFUNCTION(BlueprintCallable, Category = "ContextEffects")
	void LoadAndAddContextEffectsLibraries(AActor* OwningActor, TSet<TSoftObjectPtr<UBEContextEffectsLibrary>> ContextEffectsLibraries);

	/** */
	UFUNCTION(BlueprintCallable, Category = "ContextEffects")
	void UnloadAndRemoveContextEffectsLibraries(AActor* OwningActor);

private:

	UPROPERTY(Transient)
	TMap<AActor*, UBEContextEffectsSet*> ActiveActorEffectsMap;

};