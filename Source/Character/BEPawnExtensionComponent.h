// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Delegates/Delegate.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Templates/Casts.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BEPawnExtensionComponent.generated.h"

class UGameFrameworkComponentManager;
class UBEAbilitySystemComponent;
class UBEPawnData;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;


/**
 * UBEPawnExtensionComponent
 *
 *  すべての Pawn クラスに機能を追加して、キャラクター/乗り物などに使用できるようにするコンポーネント。
 *  これにより、他のコンポーネントの初期化が調整されます。
 */
UCLASS()
class BECORE_API UBEPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBEPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Should be called by the owning pawn to become the avatar of the ability system.
	void InitializeAbilitySystem(UBEAbilitySystemComponent* InASC, AActor* InOwnerActor);

	// Should be called by the owning pawn to remove itself as the avatar of the ability system.
	void UninitializeAbilitySystem();

	// Should be called by the owning pawn when the pawn's controller changes.
	void HandleControllerChanged();

	// Should be called by the owning pawn when the player state has been replicated.
	void HandlePlayerStateReplicated();

	// Should be called by the owning pawn when the input component is setup.
	void SetupPlayerInputComponent();

	// Register with the OnAbilitySystemInitialized delegate and broadcast if condition is already met.
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	// Register with the OnAbilitySystemUninitialized delegate.
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

protected:
	// Delegate fired when our pawn becomes the ability system's avatar actor
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	// Delegate fired when our pawn is removed as the ability system's avatar actor
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	// Pointer to the ability system component that is cached for convenience.
	UPROPERTY()
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;


	//======================================
	//	Game Feature
	//======================================
public:
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	/** The name of this component-implemented feature */
	static const FName NAME_ActorFeatureName;


	//======================================
	//	ユーティリティ
	//======================================
public:
	// Returns the pawn extension component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Pawn")
	static UBEPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UBEPawnExtensionComponent>() : nullptr); }

	UFUNCTION(BlueprintPure, Category = "Pawn")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const { return AbilitySystemComponent; }


	//======================================
	//	Pawn Data
	//======================================
public:
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	void SetPawnData(const UBEPawnData* InPawnData);

protected:
	UFUNCTION()
	void OnRep_PawnData();

protected:
	// Pawn data used to create the pawn.  Specified from a spawn function or on a placed instance.
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "Pawn")
	TObjectPtr<const UBEPawnData> PawnData;
};
