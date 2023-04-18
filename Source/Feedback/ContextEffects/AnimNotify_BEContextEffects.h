// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "AnimNotify_BEContextEffects.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct BECORE_API FBEContextEffectAnimNotifyVFXSettings
{
	GENERATED_BODY()

	// Scale to spawn the particle system at
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FX)
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);

};

/**
 *
 */
USTRUCT(BlueprintType)
struct BECORE_API FBEContextEffectAnimNotifyAudioSettings
{
	GENERATED_BODY()

	// Volume Multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float VolumeMultiplier = 1.0f;

	// Pitch Multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float PitchMultiplier = 1.0f;
};


/**
 *
 */
USTRUCT(BlueprintType)
struct BECORE_API FBEContextEffectAnimNotifyTraceSettings
{
	GENERATED_BODY()

	// Trace Channel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Visibility;

	// Vector offset from Effect Location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace)
	FVector EndTraceLocationOffset = FVector::ZeroVector;

	// Ignore this Actor when getting trace result
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace)
	bool bIgnoreActor = true;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct BECORE_API FBEContextEffectAnimNotifyPreviewSettings
{
	GENERATED_BODY()

	// If true, will attempt to match selected Surface Type to Context Tag via Project Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preview)
	bool bPreviewPhysicalSurfaceAsContext = true;

	// Surface Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preview, meta=(EditCondition="bPreviewPhysicalSurfaceAsContext"))
	TEnumAsByte<EPhysicalSurface> PreviewPhysicalSurface = EPhysicalSurface::SurfaceType_Default;

	// Preview Library
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preview, meta = (AllowedClasses = "/Script/BLACKENGINE.BEContextEffectsLibrary"))
	FSoftObjectPath PreviewContextEffectsLibrary;

	// Preview Context
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preview)
	FGameplayTagContainer PreviewContexts;
};


/**
 * 
 */
UCLASS(const, hidecategories=Object, CollapseCategories, Config = Game, meta=(DisplayName="Play Context Effects"))
class BECORE_API UAnimNotify_BEContextEffects : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_BEContextEffects();

	// Begin UObject interface
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End UObject interface

	// Begin UAnimNotify interface
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
#if WITH_EDITOR
	virtual void ValidateAssociatedAssets() override;
#endif
	// End UAnimNotify interface

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable)
	void SetParameters(FGameplayTag EffectIn, FVector LocationOffsetIn, FRotator RotationOffsetIn, 
		FBEContextEffectAnimNotifyVFXSettings VFXPropertiesIn, FBEContextEffectAnimNotifyAudioSettings AudioPropertiesIn,
		bool bAttachedIn, FName SocketNameIn, bool bPerformTraceIn, FBEContextEffectAnimNotifyTraceSettings TracePropertiesIn);
#endif


	// Effect to Play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (DisplayName = "Effect", ExposeOnSpawn = true))
	FGameplayTag Effect;

	// Location offset from the socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	FVector LocationOffset = FVector::ZeroVector;

	// Rotation offset from socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	FRotator RotationOffset = FRotator::ZeroRotator;

	// Scale to spawn the particle system at
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	FBEContextEffectAnimNotifyVFXSettings VFXProperties;

	// Scale to spawn the particle system at
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	FBEContextEffectAnimNotifyAudioSettings AudioProperties;

	// Should attach to the bone/socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttachmentProperties", meta = (ExposeOnSpawn = true))
	uint32 bAttached : 1; 	//~ Does not follow coding standard due to redirection from BP

	// SocketName to attach to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttachmentProperties", meta = (ExposeOnSpawn = true, EditCondition = "bAttached"))
	FName SocketName;

	// Will perform a trace, required for SurfaceType to Context Conversion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	uint32 bPerformTrace : 1; 	

	// Scale to spawn the particle system at
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (ExposeOnSpawn = true, EditCondition = "bPerformTrace"))
	FBEContextEffectAnimNotifyTraceSettings TraceProperties;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Config, EditAnywhere, Category = "PreviewProperties")
	uint32 bPreviewInEditor : 1;

	UPROPERTY(EditAnywhere, Category = "PreviewProperties", meta = (EditCondition = "bPreviewInEditor"))
	FBEContextEffectAnimNotifyPreviewSettings PreviewProperties;
#endif


};
