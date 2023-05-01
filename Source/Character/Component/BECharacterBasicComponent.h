// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Delegates/Delegate.h"
#include "Engine/EngineTypes.h"
#include "Templates/Casts.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BECharacterBasicComponent.generated.h"

class UBECharacterData;


/**
 * UBECharacterBasicComponent
 *
 *  Character および AbilitySystem の初期化を行う。
 *  他の InitStateSystem に登録した全てのコンポーネントの初期化状況を監視し、
 *  最終的な初期化ステートの遷移を管理する。
 */
UCLASS()
class BECORE_API UBECharacterBasicComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UBECharacterBasicComponent(const FObjectInitializer& ObjectInitializer);

	// このコンポーネントを実装する際の FeatureName
	static const FName NAME_ActorFeatureName;

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
	UFUNCTION()
	void OnRep_CharacterData();

	// この Character の基本情報
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_CharacterData, Category = "Character")
	TObjectPtr<const UBECharacterData> CharacterData;

public:
	void SetCharacterData(const UBECharacterData* InCharacterData);

	template <class T>
	const T* GetCharacterData() const { return Cast<T>(CharacterData); }


public:
	/**
	 * HandleControllerChanged
	 *
	 * このコンポーネントを所有する Character の Controller が変更されたときに Character から呼び出す
	 */
	void HandleControllerChanged();

	/**
	 * HandlePlayerStateReplicated
	 *
	 * このコンポーネントを所有する Character を所有している PlayerState がネットワークで複製されたときに Character から呼び出す
	 */
	void HandlePlayerStateReplicated();

	/**
	 * HandlePlayerInputComponentSetup
	 *
	 * このコンポーネントを所有する Character の PlayerInputComponent のセットアップが完了したときに Character から呼び出す
	 */
	void HandlePlayerInputComponentSetup();


	/**
	 * InitializeAbilitySystem
	 *
	 * このコンポーネントを所有する Character を Avater として AbilitySystem を初期化
	 */
	void InitializeAbilitySystem(UBEAbilitySystemComponent* InASC, AActor* InOwnerActor);

	/**
	 * UninitializeAbilitySystem
	 *
	 * このコンポーネントを所有する Character を AbilitySystem から除外する
	 */
	void UninitializeAbilitySystem();

	/**
	 * OnAbilitySystemInitialized_RegisterAndCall
	 *
	 * AbilitySystem の初期化完了コールバックの登録と呼び出しを行う
	 */
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/**
	 * OnAbilitySystemUninitialized_Register
	 *
	 * AbilitySystem の初期化解除完了コールバックの登録と呼び出しを行う
	 */
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

protected:
	FSimpleMulticastDelegate OnAbilitySystemInitialized;
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	UPROPERTY(Transient)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;


public:
	UFUNCTION(BlueprintPure, Category = "Character")
	static UBECharacterBasicComponent* FindCharacterBasicComponent(const APawn* Pawn);

	UFUNCTION(BlueprintPure, Category = "Character")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const { return AbilitySystemComponent; }
};
