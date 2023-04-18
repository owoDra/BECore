// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "Components/GameFrameworkComponent.h"

#include "Delegates/Delegate.h"
#include "GameFramework/Actor.h"
#include "HAL/Platform.h"
#include "UObject/UObjectGlobals.h"

#include "BEHealthComponent.generated.h"

class UBEAbilitySystemComponent;
class UBEHealthSet;
struct FFrame;
struct FGameplayEffectSpec;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBEHealth_DeathEvent, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FBEHealth_AttributeChanged, UBEHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

/////////////////////////////////////////

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

/////////////////////////////////////////

/**
 * UBEHealthComponent
 *
 *	キャラクターの Health や Death を管理するためのコンポーネント
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class BECORE_API UBEHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

	//==================================
	// 初期化
	//==================================
public:
	UBEHealthComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * AbilitySystem を初期化する。
	 */
	UFUNCTION(BlueprintCallable, Category = "BE|Health")
	void InitializeWithAbilitySystem(UBEAbilitySystemComponent* InASC);

	/**
	 * AbilitySystem 関係の参照を削除する。
	 */
	UFUNCTION(BlueprintCallable, Category = "BE|Health")
	void UninitializeFromAbilitySystem();

protected:
	virtual void OnUnregister() override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<const UBEHealthSet> HealthSet;


	//==================================
	// データ
	//==================================
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


	//==================================
	// イベント
	//==================================
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


	//==================================
	// ユーティリティ
	//==================================
public:
	/**
	 *	Actor に HealthComponent がある場合にそれを取得する。
	 */
	UFUNCTION(BlueprintPure, Category = "BE|Health")
	static UBEHealthComponent* FindHealthComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UBEHealthComponent>() : nullptr); }

	/**
	 *	HealthComponent の Owner に致死ダメージを与える。
	 */
	virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);

protected:
	void ClearGameplayTags();
};
