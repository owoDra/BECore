// Copyright owoDra

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/SoftObjectPath.h"
#include "GameplayTagContainer.h"
#include "BEContextEffectsLibrary.generated.h"

class USoundBase;
class UNiagaraSystem;

/**
 *
 */
UENUM()
enum class EContextEffectsLibraryLoadState : uint8 {
	Unloaded = 0,
	Loading = 1,
	Loaded = 2
};

/**
 *
 */
USTRUCT(BlueprintType)
struct BECORE_API FBEContextEffects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag EffectTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer Context;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowedClasses = "/Script/Engine.SoundBase, /Script/Niagara.NiagaraSystem"))
	TArray<FSoftObjectPath> Effects;

};

/**
 *
 */
UCLASS()
class BECORE_API UBEActiveContextEffects : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FGameplayTag EffectTag;

	UPROPERTY(VisibleAnywhere)
	FGameplayTagContainer Context;

	UPROPERTY(VisibleAnywhere)
	TArray<USoundBase*> Sounds;

	UPROPERTY(VisibleAnywhere)
	TArray<UNiagaraSystem*> NiagaraSystems;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FBEContextEffectLibraryLoadingComplete, TArray<UBEActiveContextEffects*>, BEActiveContextEffects);

/**
 * 
 */
UCLASS(BlueprintType)
class BECORE_API UBEContextEffectsLibrary : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FBEContextEffects> ContextEffects;

	UFUNCTION(BlueprintCallable)
	void GetEffects(const FGameplayTag Effect, const FGameplayTagContainer Context, TArray<USoundBase*>& Sounds, TArray<UNiagaraSystem*>& NiagaraSystems);

	UFUNCTION(BlueprintCallable)
	void LoadEffects();

	EContextEffectsLibraryLoadState GetContextEffectsLibraryLoadState();

private:
	void LoadEffectsInternal();

	void BEContextEffectLibraryLoadingComplete(TArray<UBEActiveContextEffects*> BEActiveContextEffects);

	UPROPERTY(Transient)
	TArray< UBEActiveContextEffects*> ActiveContextEffects;

	UPROPERTY(Transient)
	EContextEffectsLibraryLoadState EffectsLoadState = EContextEffectsLibraryLoadState::Unloaded;
};
