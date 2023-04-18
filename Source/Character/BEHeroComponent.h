// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Camera/BECameraMode.h"
#include "Input/BEMappableConfigPair.h"

#include "Containers/Array.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BEHeroComponent.generated.h"

class UGameFrameworkComponentManager;
class UInputComponent;
class UBECameraMode;
class UBEInputConfig;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;
struct FInputActionValue;


/**
 * UBEHeroComponent
 *
 *	このコンポーネントによってプレイヤーが操作する Pawn の Input や Camera を初期化される。
 *  Pawn Extention Component に依存する。
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class BECORE_API UBEHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	UBEHeroComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);


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
	// Returns the hero component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "BE|Hero")
	static UBEHeroComponent* FindHeroComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UBEHeroComponent>() : nullptr); }


	//======================================
	//	カメラ
	//======================================
public:
	void SetAbilityCameraMode(TSubclassOf<UBECameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

protected:
	TSubclassOf<UBECameraMode> DetermineCameraMode() const;

protected:
	// Camera mode set by an ability.
	TSubclassOf<UBECameraMode> AbilityCameraMode;

	// Spec handle for the last ability to set a camera mode.
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;


	//======================================
	//	インプット
	//======================================
public:
	void AddAdditionalInputConfig(const UBEInputConfig* InputConfig);
	void RemoveAdditionalInputConfig(const UBEInputConfig* InputConfig);

	/** True if this player has sent the BindInputsNow event and is prepared for bindings */
	bool IsReadyToBindInputs() const;

	static const FName NAME_BindInputsNow;

protected:
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_LookStick(const FInputActionValue& InputActionValue);
	
protected:
	/**
	 * Input Configs that should be added to this player when initalizing the input.
	 * 
	 * NOTE: You should only add to this if you do not have a game feature plugin accessible to you.
	 * If you do, then use the GameFeatureAction_AddInputConfig instead. 
	 */
	UPROPERTY(EditAnywhere)
	TArray<FMappableConfigPair> DefaultInputConfigs;

	// True when player input bindings have been applyed, will never be true for non-players
	bool bReadyToBindInputs;
};
