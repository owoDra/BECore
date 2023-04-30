// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Engine/HitResult.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathSSE.h"
#include "UObject/UObjectGlobals.h"
#include "NativeGameplayTags.h"

#include "BECharacterMovementComponent.generated.h"

class ABECharacter;
class UBEAbilitySystemComponent;
class UBECharacterMovementFragment;
class UBEMovementSet;
class UAnimMontage;
class UObject;
struct FOnAttributeChangeData;
struct FFrame;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_Walking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_NavWalking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_Falling);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_Swimming);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_Flying);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Mode_Custom);


/**
 * FBECharacterGroundInfo
 *
 *	キャラクターが立っている地面の状態
 */
USTRUCT(BlueprintType)
struct FBECharacterGroundInfo
{
	GENERATED_BODY()

	FBECharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};


/**
 * UBECharacterMovementComponent
 *
 *	Character の Movement 関係の処理を担当するコンポーネント
 */
UCLASS()
class BECORE_API UBECharacterMovementComponent : public UCharacterMovementComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	/**
	 * FSavedMove_BECharacter
	 *
	 *	ネットワークにレプリケートするために移動データを保存するためのクラス
	 */
	class FSavedMove_BECharacter : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;

		enum CompressedFlags
		{
			FLAG_Run		= 0x10,
			FLAG_Target		= 0x20,
			FLAG_Custom_2	= 0x40,
			FLAG_Custom_3	= 0x80,
		};

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* Character) override;

		// Flags
		uint8 Saved_bWantsToRun		: 1;
		uint8 Saved_bWantsToTarget	: 1;
	};

	/**
	 * FNetworkPredictionData_Client_BECharacter
	 */
	class FNetworkPredictionData_Client_BECharacter : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_BECharacter(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		// Allocates a new copy of our custom saved move
		virtual FSavedMovePtr AllocateNewMove() override;
	};
	
	UBECharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	// このコンポーネントを実装する際の FeatureName
	static const FName NAME_ActorFeatureName;

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeWithAbilitySystem(UBEAbilitySystemComponent* InASC);
	void UninitializeFromAbilitySystem();
	void InitializeGameplayTags();

	UPROPERTY(Transient)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<const UBEMovementSet> MovementSet;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;

	UPROPERTY(Transient)
	FBECharacterGroundInfo CachedGroundInfo;

	
public:
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface


protected:
	virtual void HandleGravityScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleGroundFrictionChanged(const FOnAttributeChangeData& ChangeData);

	virtual void HandleOverallSpeedMultiplierChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleWalkSpeedChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleWalkSpeedCrouchedChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleWalkSpeedRunningChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleWalkSpeedTargetingChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleSwimSpeedChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleFlySpeedChanged(const FOnAttributeChangeData& ChangeData);

	virtual void HandleJumpPowerChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleAirControlChanged(const FOnAttributeChangeData& ChangeData);


public:
	void SetReplicatedAcceleration(const FVector& InAcceleration);
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void SimulateMovement(float DeltaTime) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual bool CanAttemptJump() const override;
	virtual bool IsMovingOnGround() const override;
	virtual bool IsMovingInAir() const;

	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	virtual float GetSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	virtual float GetSpeed2D() const;

protected:
	void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations);


public:
	uint8 bWantsToRun : 1;

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool IsRunning() const;

	virtual bool CanRunInCurrentState() const;

	virtual void Run(bool bClientSimulation);
	virtual void UnRun(bool bClientSimulation);

public:
	uint8 bWantsToTarget : 1;

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool IsTargeting() const;

	virtual bool CanTargetInCurrentState() const;

	virtual void Target(bool bClientSimulation);
	virtual void UnTarget(bool bClientSimulation);


public:
	ABECharacter* GetBECharacterOwner() const { return Cast<ABECharacter>(CharacterOwner); }

	UFUNCTION(BlueprintPure, Category = "Character")
	static UBECharacterMovementComponent* FindCharacterMovementComponent(const ABECharacter* Character);

	/**
	 * GetGroundInfo
	 * 
	 *  現在の地面の情報を返します。古い場合は、これを呼び出すと地面情報が更新される
	 */
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	const FBECharacterGroundInfo& GetGroundInfo();


	//==========================================================
	//	Movement Settings
	// 
	// ここからは Movement 処理に関係する定数の定義を行う
	//==========================================================

public:
	// 移動速度全体に影響する速度倍率
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, meta = (DisplayAfter = "GravityScale", ClampMin = "0", UIMin = "0", ForceUnits = "x"))
	float OverallMaxSpeedMultiplier = 1.0;

	// Movement Mode が Walking かつ走り状態の最大移動速度
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (DisplayAfter = "MaxWalkSpeedCrouched", ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MaxWalkSpeedRunning = 550;

	// Movement Mode が Walking かつターゲット状態の最大移動速度
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (DisplayAfter = "MaxWalkSpeedCrouched", ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MaxWalkSpeedTargeting = 250;

	// CustomMovementMode の Movement 処理
	// 配列の順番が CustomMovement の番号と一致する
	UPROPERTY(EditDefaultsOnly, Category = "Inventory", Instanced)
	TArray<TObjectPtr<UBECharacterMovementFragment>> Fragments;
};
