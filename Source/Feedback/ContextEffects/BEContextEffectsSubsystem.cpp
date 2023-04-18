// Copyright Eigi Chin

#include "BEContextEffectsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "BEContextEffectsLibrary.h"


void UBEContextEffectsSubsystem::SpawnContextEffects(
	const AActor* SpawningActor
	, USceneComponent* AttachToComponent
	, const FName AttachPoint
	, const FVector LocationOffset
	, const FRotator RotationOffset
	, FGameplayTag Effect
	, FGameplayTagContainer Contexts
	, TArray<UAudioComponent*>& AudioOut
	, TArray<UNiagaraComponent*>& NiagaraOut
	, FVector VFXScale
	, float AudioVolume
	, float AudioPitch)
{
	// First determine if this Actor has a matching Set of Libraries
	if (UBEContextEffectsSet** EffectsLibrariesSetPtr = ActiveActorEffectsMap.Find(SpawningActor))
	{
		// Validate the pointers from the Map Find
		if (UBEContextEffectsSet* EffectsLibraries = *EffectsLibrariesSetPtr)
		{
			// Prepare Arrays for Sounds and Niagara Systems
			TArray<USoundBase*> TotalSounds;
			TArray<UNiagaraSystem*> TotalNiagaraSystems;

			// Cycle through Effect Libraries
			for (UBEContextEffectsLibrary* EffectLibrary : EffectsLibraries->BEContextEffectsLibraries)
			{
				// Check if the Effect Library is valid and data Loaded
				if (EffectLibrary && EffectLibrary->GetContextEffectsLibraryLoadState() == EContextEffectsLibraryLoadState::Loaded)
				{
					// Set up local list of Sounds and Niagara Systems
					TArray<USoundBase*> Sounds;
					TArray<UNiagaraSystem*> NiagaraSystems;

					// Get Sounds and Niagara Systems
					EffectLibrary->GetEffects(Effect, Contexts, Sounds, NiagaraSystems);

					// Append to accumulating array
					TotalSounds.Append(Sounds);
					TotalNiagaraSystems.Append(NiagaraSystems);
				}
				else if (EffectLibrary && EffectLibrary->GetContextEffectsLibraryLoadState() == EContextEffectsLibraryLoadState::Unloaded)
				{
					// Else load effects
					EffectLibrary->LoadEffects();
				}
			}

			// Cycle through found Sounds
			for (USoundBase* Sound : TotalSounds)
			{
				// Spawn Sounds Attached, add Audio Component to List of ACs
				UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAttached(Sound, AttachToComponent, AttachPoint, LocationOffset, RotationOffset, EAttachLocation::KeepRelativeOffset,
					false, AudioVolume, AudioPitch, 0.0f, nullptr, nullptr, true);

				AudioOut.Add(AudioComponent);
			}

			// Cycle through found Niagara Systems
			for (UNiagaraSystem* NiagaraSystem : TotalNiagaraSystems)
			{
				// Spawn Niagara Systems Attached, add Niagara Component to List of NCs
				UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, AttachToComponent, AttachPoint, LocationOffset,
					RotationOffset, VFXScale, EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None, true, true);

				NiagaraOut.Add(NiagaraComponent);
			}
		}
	}
}

bool UBEContextEffectsSubsystem::GetContextFromSurfaceType(
	TEnumAsByte<EPhysicalSurface> PhysicalSurface, FGameplayTag& Context)
{
	// Get Project Settings
	if (const UBEContextEffectsSettings* ProjectSettings = GetDefault<UBEContextEffectsSettings>())
	{
		// Find which Gameplay Tag the Surface Type is mapped to
		if (const FGameplayTag* GameplayTagPtr = ProjectSettings->SurfaceTypeToContextMap.Find(PhysicalSurface))
		{
			Context = *GameplayTagPtr;
		}
	}

	// Return true if Context is Valid
	return Context.IsValid();
}

void UBEContextEffectsSubsystem::LoadAndAddContextEffectsLibraries(AActor* OwningActor,
	TSet<TSoftObjectPtr<UBEContextEffectsLibrary>> ContextEffectsLibraries)
{
	// Early out if Owning Actor is invalid or if the associated Libraries is 0 (or less)
	if (OwningActor == nullptr || ContextEffectsLibraries.Num() <= 0)
	{
		return;
	}

	// Create new Context Effect Set
	UBEContextEffectsSet* EffectsLibrariesSet = NewObject<UBEContextEffectsSet>(this);

	// Cycle through Libraries getting Soft Obj Refs
	for (const TSoftObjectPtr<UBEContextEffectsLibrary>& ContextEffectSoftObj : ContextEffectsLibraries)
	{
		// Load Library Assets from Soft Obj refs
		// TODO Support Async Loading of Asset Data
		if (UBEContextEffectsLibrary* EffectsLibrary = ContextEffectSoftObj.LoadSynchronous())
		{
			// Call load on valid Libraries
			EffectsLibrary->LoadEffects();

			// Add new library to Set
			EffectsLibrariesSet->BEContextEffectsLibraries.Add(EffectsLibrary);
		}
	}

	// Update Active Actor Effects Map
	ActiveActorEffectsMap.Emplace(OwningActor, EffectsLibrariesSet);
}

void UBEContextEffectsSubsystem::UnloadAndRemoveContextEffectsLibraries(AActor* OwningActor)
{
	// Early out if Owning Actor is invalid
	if (OwningActor == nullptr)
	{
		return;
	}

	// Remove ref from Active Actor/Effects Set Map
	ActiveActorEffectsMap.Remove(OwningActor);
}
