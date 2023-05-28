// Copyright Eigi Chin

#pragma once

#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Delegates/Delegate.h"
#include "HAL/Platform.h"
#include "UObject/UObjectGlobals.h"
#include "NativeGameplayTags.h"

#include "BEPawnHealthComponent.generated.h"

class UBEAbilitySystemComponent;
class UBEHealthSet;
struct FFrame;
struct FGameplayEffectSpec;
struct FOnAttributeChangeData;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dying);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dead);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Death);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBEHealth_DeathEvent, APawn*, OwningPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FBEHealth_AttributeChanged, UBEPawnHealthComponent*, HealthComponent, float, OldValue, float, NewValue, APawn*, Instigator);


/**
 * EBEDeathState
 *
 *	現在の死亡ステート.
 */
UENUM(BlueprintType)
enum class EBEDeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};


/**
 * UBEPawnHealthComponent
 *
 *	キャラクターの Health や Death を管理するためのコンポーネント
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class BECORE_API UBEPawnHealthComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UBEPawnHealthComponent(const FObjectInitializer& ObjectInitializer);

	// このコンポーネントを実装する際の FeatureName
	static const FName NAME_ActorFeatureName;

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeWithAbilitySystem(UBEAbilitySystemComponent* InASC);
	void UninitializeFromAbilitySystem();
	void ClearGameplayTags();


public:
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface


protected:
	UPROPERTY(Transient)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<const UBEHealthSet> HealthSet;

	UPROPERTY(Transient)
	TObjectPtr<const UBECombatSet> CombatSet;


public:
	// 初期最大HP
	UPROPERTY(Category = "BE|Health", EditAnywhere, BlueprintReadOnly)
	float DefaultMaxHealth;

	// 初期最大シールド
	UPROPERTY(Category = "BE|Health", EditAnywhere, BlueprintReadOnly)
	float DefaultMaxShield;

	// 初期HP
	UPROPERTY(Category = "BE|Health", EditAnywhere, BlueprintReadOnly)
	float DefaultHealth;

	// 初期HP
	UPROPERTY(Category = "BE|Health", EditAnywhere, BlueprintReadOnly)
	float DefaultShield;

public:
	// 現在の Health を取得
	UFUNCTION(BlueprintCallable, Category = "BE|Health")
	float GetHealth() const;

	// 現在の Shield を取得
	UFUNCTION(BlueprintCallable, Category = "BE|Health")
	float GetShield() const;

	// 現在の MaxHealth を取得
	UFUNCTION(BlueprintCallable, Category = "BE|Health")
	float GetMaxHealth() const;

	// 現在の MaxShield を取得
	UFUNCTION(BlueprintCallable, Category = "BE|Health")
	float GetMaxShield() const;

	// 現在の Health と Shield を合わせた値を取得
	UFUNCTION(BlueprintCallable, Category = "BE|Health")
	float GetTotalHealth() const;

	// 現在の MaxHealth と MaxShield を合わせた値を取得
	UFUNCTION(BlueprintCallable, Category = "BE|Health")
	float GetTotalMaxHealth() const;

public:
	UFUNCTION(BlueprintCallable, Category = "BE|Health")
	EBEDeathState GetDeathState() const { return DeathState; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "BE|Health", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsDeadOrDying() const { return (DeathState > EBEDeathState::NotDead); }

protected:
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EBEDeathState DeathState;


public:
	// Health が変わったときのデリゲート
	UPROPERTY(BlueprintAssignable)
	FBEHealth_AttributeChanged OnHealthChanged;

	// Shield が変わったときのデリゲート
	UPROPERTY(BlueprintAssignable)
	FBEHealth_AttributeChanged OnShieldChanged;

	// MaxHealth が変わったときのデリゲート
	UPROPERTY(BlueprintAssignable)
	FBEHealth_AttributeChanged OnMaxHealthChanged;

	// MaxShield が変わったときのデリゲート
	UPROPERTY(BlueprintAssignable)
	FBEHealth_AttributeChanged OnMaxShieldChanged;

	// Delegate fired when the death sequence has started.
	UPROPERTY(BlueprintAssignable)
	FBEHealth_DeathEvent OnDeathStarted;

	// Delegate fired when the death sequence has finished.
	UPROPERTY(BlueprintAssignable)
	FBEHealth_DeathEvent OnDeathFinished;

public:
	// Death 処理が開始した時に実行
	virtual void StartDeath();

	// Death 処理が終了した時に実行
	virtual void FinishDeath();
	
protected:
	virtual void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleShieldChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMaxHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMaxShieldChanged(const FOnAttributeChangeData& ChangeData);

	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);

	UFUNCTION()
	virtual void OnRep_DeathState(EBEDeathState OldDeathState);


public:
	UFUNCTION(BlueprintPure, Category = "Character")
	static UBEPawnHealthComponent* FindPawnHealthComponent(const APawn* Pawn);

	/**
	 *	HealthComponent の Owner に致死ダメージを与える。
	 */
	virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);
};
