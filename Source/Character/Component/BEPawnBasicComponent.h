// Copyright owoDra

#pragma once

#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Delegates/Delegate.h"
#include "UObject/NameTypes.h"
#include "UObject/UObjectGlobals.h"

#include "BEPawnBasicComponent.generated.h"

class AActor;
class APawn;
class UBEPawnData;
class UBEAbilitySystemComponent;


/**
 * UBEPawnBasicComponent
 *
 *  Pawn または Character と AbilitySystem の初期化を行う。
 *  他の InitStateSystem に登録した全てのコンポーネントの初期化状況を監視し、
 *  最終的な初期化ステートの遷移を管理する。
 */
UCLASS()
class BECORE_API UBEPawnBasicComponent 
	: public UPawnComponent
	, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UBEPawnBasicComponent(const FObjectInitializer& ObjectInitializer);

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
	void OnRep_PawnData();

	// このコンポーネントを所有する Pawn または Character の基本情報
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "Pawn")
	TObjectPtr<const UBEPawnData> PawnData;

public:
	const UBEPawnData* GetPawnData() const { return PawnData; }

	/**
	 * SetPawnData
	 *
	 * このコンポーネントを所有する Pawn または Character の基本情報である PawnData を設定する
	 * 通常これはスポーン時(BeginPlayが呼び出される前)に GameMode などによって実行される
	 */
	void SetPawnData(const UBEPawnData* InPawnData);
	

public:
	/**
	 * HandleControllerChanged
	 *
	 * このコンポーネントを所有する Pawn または Character の Controller が変更されたときに Character から呼び出す
	 */
	void HandleControllerChanged();

	/**
	 * HandlePlayerStateReplicated
	 *
	 * このコンポーネントを所有する Pawn または Character を所有している PlayerState がネットワークで複製されたときに Character から呼び出す
	 */
	void HandlePlayerStateReplicated();

	/**
	 * HandlePlayerInputComponentSetup
	 *
	 * このコンポーネントを所有する Pawn または Character の PlayerInputComponent のセットアップが完了したときに Character から呼び出す
	 */
	void HandlePlayerInputComponentSetup();

	/**
	 * InitializeAnimLayers
	 *
	 * このコンポーネントを所有する Pawn または Character の DefaultAnimLayer を設定する
	 */
	void InitializeAnimLayers(APawn* InOwningPawn);

	/**
	 * InitializeAbilitySystem
	 *
	 * このコンポーネントを所有する Pawn または Character を Avater として AbilitySystem を初期化
	 */
	void InitializeAbilitySystem(UBEAbilitySystemComponent* InASC, AActor* InOwnerActor);

	/**
	 * UninitializeAbilitySystem
	 *
	 * このコンポーネントを所有する Pawn または Character を AbilitySystem から除外する
	 */
	void UninitializeAbilitySystem();

	/**
	 * OnAbilitySystemInitialized_Register
	 *
	 * AbilitySystem が初期化されたときのコールバックに登録する
	 */
	void OnAbilitySystemInitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

	/**
	 * OnAbilitySystemUninitialized_Register
	 *
	 * Pawn が初期化されたときのコールバックに登録する
	 */
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

	/**
	 * OnPawnInitialized_Register
	 *
	 * AbilitySystem が初期化解除されたときのコールバックに登録する
	 */
	void OnPawnInitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

protected:
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	FSimpleMulticastDelegate OnPawnInitialized;

	UPROPERTY(Transient)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;


public:
	UFUNCTION(BlueprintPure, Category = "Character")
	static UBEPawnBasicComponent* FindPawnBasicComponent(const APawn* Pawn);

	UFUNCTION(BlueprintPure, Category = "Character")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const { return AbilitySystemComponent; }
};
