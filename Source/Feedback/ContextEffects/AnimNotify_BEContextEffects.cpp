// Copyright Eigi Chin

#include "AnimNotify_BEContextEffects.h"
#include "BEContextEffectComponent.h"
#include "BEContextEffectsInterface.h"
#include "UObject/UObjectBaseUtility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "BEContextEffectsSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "BEContextEffectsLibrary.h"
#include "Kismet/GameplayStatics.h"


UAnimNotify_BEContextEffects::UAnimNotify_BEContextEffects()
{
}

void UAnimNotify_BEContextEffects::PostLoad()
{
	Super::PostLoad();
}

#if WITH_EDITOR
void UAnimNotify_BEContextEffects::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

FString UAnimNotify_BEContextEffects::GetNotifyName_Implementation() const
{
	// If the Effect Tag is valid, pass the string name to the notify name
	if (Effect.IsValid())
	{
		return Effect.ToString();
	}

	return Super::GetNotifyName_Implementation();
}

void UAnimNotify_BEContextEffects::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		// Make sure both MeshComp and Owning Actor is valid
		if (AActor* OwningActor = MeshComp->GetOwner())
		{
			// Prepare Trace Data
			bool bHitSuccess = false;
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;

			if (TraceProperties.bIgnoreActor)
			{
				QueryParams.AddIgnoredActor(OwningActor);
			}

			QueryParams.bReturnPhysicalMaterial = true;

			if (bPerformTrace)
			{
				// If trace is needed, set up Start Location to Attached
				FVector TraceStart = bAttached ? MeshComp->GetSocketLocation(SocketName) : MeshComp->GetComponentLocation();

				// Make sure World is valid
				if (UWorld* World = OwningActor->GetWorld())
				{
					// Call Line Trace, Pass in relevant properties
					bHitSuccess = World->LineTraceSingleByChannel(HitResult, TraceStart, (TraceStart + TraceProperties.EndTraceLocationOffset),
						TraceProperties.TraceChannel, QueryParams, FCollisionResponseParams::DefaultResponseParam);
				}
			}

			// Prepare Contexts in advance
			FGameplayTagContainer Contexts;

			// Set up Array of Objects that implement the Context Effects Interface
			TArray<UObject*> BEContextEffectImplementingObjects;

			// Determine if the Owning Actor is one of the Objects that implements the Context Effects Interface
			if (OwningActor->Implements<UBEContextEffectsInterface>())
			{
				// If so, add it to the Array
				BEContextEffectImplementingObjects.Add(OwningActor);
			}

			// Cycle through Owning Actor's Components and determine if any of them is a Component implementing the Context Effect Interface
			for (const auto Component : OwningActor->GetComponents())
			{
				if (Component)
				{
					// If the Component implements the Context Effects Interface, add it to the list
					if (Component->Implements<UBEContextEffectsInterface>())
					{
						BEContextEffectImplementingObjects.Add(Component);
					}
				}
			}

			// Cycle through all objects implementing the Context Effect Interface
			for (UObject* BEContextEffectImplementingObject : BEContextEffectImplementingObjects)
			{
				if (BEContextEffectImplementingObject)
				{
					// If the object is still valid, Execute the AnimMotionEffect Event on it, passing in relevant data
					IBEContextEffectsInterface::Execute_AnimMotionEffect(BEContextEffectImplementingObject,
						(bAttached ? SocketName : FName("None")),
						Effect, MeshComp, LocationOffset, RotationOffset,
						Animation, bHitSuccess, HitResult, Contexts, VFXProperties.Scale,
						AudioProperties.VolumeMultiplier, AudioProperties.PitchMultiplier);
				}
			}

#if WITH_EDITORONLY_DATA
			// This is for Anim Editor previewing, it is a deconstruction of the calls made by the Interface and the Subsystem
			if (bPreviewInEditor)
			{
				UWorld* World = OwningActor->GetWorld();

				// Get the world, make sure it's an Editor Preview World
				if (World && World->WorldType == EWorldType::EditorPreview)
				{
					// Add Preview contexts if necessary
					Contexts.AppendTags(PreviewProperties.PreviewContexts);

					// Convert given Surface Type to Context and Add it to the Contexts for this Preview
					if (PreviewProperties.bPreviewPhysicalSurfaceAsContext)
					{
						TEnumAsByte<EPhysicalSurface> PhysicalSurfaceType = PreviewProperties.PreviewPhysicalSurface;

						if (const UBEContextEffectsSettings* BEContextEffectsSettings = GetDefault<UBEContextEffectsSettings>())
						{
							if (const FGameplayTag* SurfaceContextPtr = BEContextEffectsSettings->SurfaceTypeToContextMap.Find(PhysicalSurfaceType))
							{
								FGameplayTag SurfaceContext = *SurfaceContextPtr;

								Contexts.AddTag(SurfaceContext);
							}
						}
					}

					// Libraries are soft referenced, so you will want to try to load them now
					// TODO Async Asset Loading
					if (UObject* EffectsLibrariesObj = PreviewProperties.PreviewContextEffectsLibrary.TryLoad())
					{
						// Check if it is in fact a UBEContextEffectLibrary type
						if (UBEContextEffectsLibrary* EffectLibrary = Cast<UBEContextEffectsLibrary>(EffectsLibrariesObj))
						{
							// Prepare Sounds and Niagara System Arrays
							TArray<USoundBase*> TotalSounds;
							TArray<UNiagaraSystem*> TotalNiagaraSystems;

							// Attempt to load the Effect Library content (will cache in Transient data on the Effect Library Asset)
							EffectLibrary->LoadEffects();

							// If the Effect Library is valid and marked as Loaded, Get Effects from it
							if (EffectLibrary && EffectLibrary->GetContextEffectsLibraryLoadState() == EContextEffectsLibraryLoadState::Loaded)
							{
								// Prepare local arrays
								TArray<USoundBase*> Sounds;
								TArray<UNiagaraSystem*> NiagaraSystems;

								// Get the Effects
								EffectLibrary->GetEffects(Effect, Contexts, Sounds, NiagaraSystems);

								// Append to the accumulating arrays
								TotalSounds.Append(Sounds);
								TotalNiagaraSystems.Append(NiagaraSystems);
							}

							// Cycle through Sounds and call Spawn Sound Attached, passing in relevant data
							for (USoundBase* Sound : TotalSounds)
							{
								UGameplayStatics::SpawnSoundAttached(Sound, MeshComp, (bAttached ? SocketName : FName("None")), LocationOffset, RotationOffset, EAttachLocation::KeepRelativeOffset,
									false, AudioProperties.VolumeMultiplier, AudioProperties.PitchMultiplier, 0.0f, nullptr, nullptr, true);
							}

							// Cycle through Niagara Systems and call Spawn System Attached, passing in relevant data
							for (UNiagaraSystem* NiagaraSystem : TotalNiagaraSystems)
							{
								UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, MeshComp, (bAttached ? SocketName : FName("None")), LocationOffset,
									RotationOffset, VFXProperties.Scale, EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::None, true, true);
							}
						}
					}
						
				}
			}
#endif

		}
	}
}

#if WITH_EDITOR
void UAnimNotify_BEContextEffects::ValidateAssociatedAssets()
{
	Super::ValidateAssociatedAssets();
}

void UAnimNotify_BEContextEffects::SetParameters(FGameplayTag EffectIn, FVector LocationOffsetIn, FRotator RotationOffsetIn,
	FBEContextEffectAnimNotifyVFXSettings VFXPropertiesIn, FBEContextEffectAnimNotifyAudioSettings AudioPropertiesIn,
	bool bAttachedIn, FName SocketNameIn, bool bPerformTraceIn, FBEContextEffectAnimNotifyTraceSettings TracePropertiesIn)
{
	Effect = EffectIn;
	LocationOffset = LocationOffsetIn;
	RotationOffset = RotationOffsetIn;
	VFXProperties.Scale = VFXPropertiesIn.Scale;
	AudioProperties.PitchMultiplier = AudioPropertiesIn.PitchMultiplier;
	AudioProperties.VolumeMultiplier = AudioPropertiesIn.VolumeMultiplier;
	bAttached = bAttachedIn;
	SocketName = SocketNameIn;
	bPerformTrace = bPerformTraceIn;
	TraceProperties.EndTraceLocationOffset = TracePropertiesIn.EndTraceLocationOffset;
	TraceProperties.TraceChannel = TracePropertiesIn.TraceChannel;
	TraceProperties.bIgnoreActor = TracePropertiesIn.bIgnoreActor;

}
#endif
