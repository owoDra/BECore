// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Input/BEMappableConfigPair.h"

#include "Containers/Array.h"
#include "Engine/EngineTypes.h"
#include "GameplayAbilitySpec.h"
#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BEPawnPlayableComponent.generated.h"

class UGameFrameworkComponentManager;
class UInputComponent;
class UBEInputConfig;
class APawn;
class UObject;
struct FActorInitStateChangedParams;
struct FGameplayTag;
struct FInputActionValue;


/**
 * UBEPawnPlayableComponent
 *
 *	このコンポーネントによってプレイヤーが操作する Pawn の Input や Camera を初期化される。
 *  Pawn Extention Component に依存する。
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class BECORE_API UBEPawnPlayableComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UBEPawnPlayableComponent(const FObjectInitializer& ObjectInitializer);

	// このコンポーネントを実装する際の FeatureName
	static const FName NAME_ActorFeatureName;

	static const FName NAME_BindInputsNow;

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface


protected:
	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);

public:
	void AddAdditionalInputConfig(const UBEInputConfig* InputConfig);
	void RemoveAdditionalInputConfig(const UBEInputConfig* InputConfig);

	// プレイヤーの入力バインドの準備が完了しているか
	bool IsReadyToBindInputs() const;

protected:
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_LookStick(const FInputActionValue& InputActionValue);

protected:
	// プレイヤーの入力バインドの準備が完了しているか
	// PlayerController 以外の場合は true になることはない
	bool bReadyToBindInputs;
	

public:
	UFUNCTION(BlueprintPure, Category = "Character")
	static UBEPawnPlayableComponent* FindCharacterPlayableComponent(const APawn* Pawn);
};
