// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "ModularCharacter.h"
#include "Teams/BETeamAgentInterface.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"

#include "GenericTeamAgentInterface.h"
#include "Containers/Array.h"
#include "Engine/EngineTypes.h"
#include "HAL/Platform.h"
#include "UObject/UObjectGlobals.h"

#include "BECharacter.generated.h"

class ABEPlayerController;
class ABEPlayerState;
class UBEAbilitySystemComponent;
class UAbilitySystemComponent;
class UBEPawnExtensionComponent;
class UBEHealthComponent;
class UBECameraComponent;
class AActor;
class AController;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UInputComponent;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimingChangedSignature, bool, isAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRunningChangedSignature, bool, isRunning);

////////////////////////////////////////////////

/**
 * FBEReplicatedAcceleration: 加速度データの圧縮用
 */
USTRUCT()
struct FBEReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0;		// XY軸の方向 [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0;		// XY軸の加速度 [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0;				// Z加速度 [-MaxAcceleration, MaxAcceleration]
};


////////////////////////////////////////////////

/**
 * ABECharacter
 *
 *	このプロジェクトのベースとなるキャラクタークラス。
 *	Pawn Componentへのイベント送信を行う。
 * 
 *	!!新しい機能はできる限りPawn Componentから追加すること!!
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class BECORE_API ABECharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface, public IBETeamAgentInterface
{
	GENERATED_BODY()

	//======================================
	//	初期化
	//======================================
public:
	ABECharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	virtual void NotifyControllerChanged() override;

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void OnAbilitySystemInitialized();
	virtual void OnAbilitySystemUninitialized();

	void InitializeGameplayTags();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBEPawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBEHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBECameraComponent> CameraComponent;


	//======================================
	//	チーム
	//======================================
public:
	//~IBETeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnBETeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IBETeamAgentInterface interface

protected:
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}

private:
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
		FGenericTeamId MyTeamID;

	UPROPERTY()
		FOnBETeamIndexChangedDelegate OnTeamChangedDelegate;

	UFUNCTION()
		void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
		void OnRep_MyTeamID(FGenericTeamId OldTeamID);


	//======================================
	//	キャラクター
	//======================================
protected:
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
		virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
		virtual void OnDeathFinished(AActor* OwningActor);

	// Called when the death sequence for the character has completed
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDeathFinished"))
		void K2_OnDeathFinished();

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

	void DisableMovementAndCollision();
	void DestroyDueToDeath();
	void UninitAndDestroy();

private:
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
		FBEReplicatedAcceleration ReplicatedAcceleration;
	
	UFUNCTION()
		void OnRep_ReplicatedAcceleration();

public:
	UPROPERTY(EditDefaultsOnly)
		bool UpdateMeshPositionWhenCrouch = true;


	//======================================
	//	しゃがみ
	//======================================
protected:
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;


	//======================================
	//	ジャンプ
	//======================================
public:
	virtual void ResetJumpState() override;
	virtual void ClearJumpInput(float DeltaTime) override;

protected:
	virtual bool CanJumpInternal_Implementation() const;


	//======================================
	//	走り
	//======================================
public:
	/** Set by character movement to specify that this Character is currently running. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsRunning, Category = "Character|Run")
		uint32 bIsRunning : 1;

	UFUNCTION()
		virtual void OnRep_IsRunning();

	UFUNCTION(BlueprintCallable, Category = "Character|Run")
		void Run();

	UFUNCTION(BlueprintCallable, Category = "Character|Run")
		void UnRun();

	virtual void OnStartRun();
	virtual void OnEndRun();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartRun", ScriptName = "OnStartRun"))
		void K2_OnStartRun();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndRun", ScriptName = "OnEndRun"))
		void K2_OnEndRun();

	UPROPERTY(BlueprintAssignable, Category = "Character|Run")
		FRunningChangedSignature OnRunChanged;


	//======================================
	//	エイム
	//======================================
public:
	/** Set by character movement to specify that this Character is currently aiming. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsAiming, Category = "Character|Aim")
		uint32 bIsAiming : 1;

	UFUNCTION()
		virtual void OnRep_IsAiming();

	UFUNCTION(BlueprintCallable, Category = "Character|Aim")
		void Aim();

	UFUNCTION(BlueprintCallable, Category = "Character|Aim")
		void UnAim();

	virtual void OnStartAim();
	virtual void OnEndAim();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartAim", ScriptName = "OnStartAim"))
		void K2_OnStartAim();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndAim", ScriptName = "OnEndAim"))
		void K2_OnEndAim();

	UPROPERTY(BlueprintAssignable, Category = "Character|Aim")
		FAimingChangedSignature OnAimChanged;


	//======================================
	//	スライド
	//======================================
public:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartSlide", ScriptName = "OnStartSlide"))
		void K2_OnStartSlide();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndSlide", ScriptName = "OnEndSlide"))
		void K2_OnEndSlide();


	//======================================
	//	壁登り
	//======================================
public:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartClimb", ScriptName = "OnStartClimb"))
		void K2_OnStartClimb();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndClimb", ScriptName = "OnEndClimb"))
		void K2_OnEndClimb();


	//======================================
	//	壁走り
	//======================================
public:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartWallRun", ScriptName = "OnStartWallRun"))
		void K2_OnStartWallRun();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndWallRun", ScriptName = "OnEndWallRun"))
		void K2_OnEndWallRun();


	//======================================
	//	ユーティリティ
	//======================================
public:
	UFUNCTION(BlueprintCallable, Category = "Character")
	ABEPlayerController* GetBEPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	ABEPlayerState* GetBEPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	FCollisionQueryParams GetIgnoreCharacterParams() const;
};
