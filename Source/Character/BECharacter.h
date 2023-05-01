// Copyright Eigi Chin

#pragma once

#include "ModularCharacter.h"
#include "Team/BETeamAgentInterface.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"

#include "GenericTeamAgentInterface.h"
#include "Containers/Array.h"
#include "Engine/EngineTypes.h"
#include "HAL/Platform.h"
#include "UObject/UObjectGlobals.h"
#include "NativeGameplayTags.h"

#include "BECharacter.generated.h"

class UBECharacterBasicComponent;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Crouching);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Running);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Targeting);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingChangedSignature, bool, isTargeting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRunningChangedSignature, bool, isRunning);


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


/**
 * ABECharacter
 *
 *	このプロジェクトのベースとなるキャラクタークラス。
 *	新しい機能はできる限りPawn Componentから追加すること。
 */
UCLASS()
class BECORE_API ABECharacter 
	: public AModularCharacter
	, public IAbilitySystemInterface
	, public IGameplayCueInterface
	, public IGameplayTagAssetInterface
	, public IBETeamAgentInterface
{
	GENERATED_BODY()

public:
	ABECharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBECharacterBasicComponent> CharacterBasicComponent;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FBEReplicatedAcceleration ReplicatedAcceleration;

	UPROPERTY(EditDefaultsOnly)
	bool UpdateMeshPositionWhenCrouch = true;

	UFUNCTION()
	void OnRep_ReplicatedAcceleration();

public:
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void NotifyControllerChanged() override;

	virtual void Reset() override;

	void DisableMovementAndCollision();
	void UninitAndDestroy();


protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;


protected:
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
public:
	virtual void ResetJumpState() override;
	virtual void ClearJumpInput(float DeltaTime) override;
protected:
	virtual bool CanJumpInternal_Implementation() const;


public:
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

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsTargeting, Category = "Character|Target")
	uint32 bIsTargeting : 1;

	UFUNCTION()
	virtual void OnRep_IsTargeting();

	UFUNCTION(BlueprintCallable, Category = "Character|Target")
	void Target();

	UFUNCTION(BlueprintCallable, Category = "Character|Target")
	void UnTarget();

	virtual void OnStartTarget();
	virtual void OnEndTarget();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartTarget", ScriptName = "OnStartTarget"))
	void K2_OnStartTarget();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndTarget", ScriptName = "OnEndTarget"))
	void K2_OnEndTarget();

	UPROPERTY(BlueprintAssignable, Category = "Character|Target")
	FTargetingChangedSignature OnTargetChanged;


public:
	//~IBETeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnBETeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IBETeamAgentInterface interface

protected:
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const;

private:
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnBETeamIndexChangedDelegate OnTeamChangedDelegate;

	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);


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
