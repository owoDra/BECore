// Copyright owoDra

#pragma once

#include "ModularCharacter.h"

#include "Team/BETeamAgentInterface.h"
#include "BEPawnMeshAssistInterface.h"

#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GenericTeamAgentInterface.h"
#include "Containers/Array.h"
#include "Engine/EngineTypes.h"
#include "HAL/Platform.h"
#include "UObject/UObjectGlobals.h"

#include "BECharacter.generated.h"

class UBEPawnBasicComponent;
class UBECharacterMovementComponent;
class UBECharacterAnimInstance;
struct FMovementBaseState;

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
	, public IBEPawnMeshAssistInterface
{
	GENERATED_BODY()
public:
	explicit ABECharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

protected:
	//
	// Character の基本的なデータやコンポーネントの初期化を管理するためのコンポーネント
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UBEPawnBasicComponent> CharacterBasic;

	//
	// Character の移動を管理するコンポーネント。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UBECharacterMovementComponent> BECharacterMovement;


	////////////////////////////////////////////////
	// Replication
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostNetReceiveLocationAndRotation() override;
	virtual void OnRep_ReplicatedBasedMovement() override;


	////////////////////////////////////////////////
	// Initialize and Uninitialize
public:
	virtual void PostInitializeComponents() override;

	virtual void Reset() override;

	virtual void NotifyControllerChanged() override;

private:
	void DisableMovementAndCollision();
	void UninitAndDestroy();

protected:
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void OnPawnBasicInitialized() {}
	virtual void OnAbilitySystemInitialized() {}
	virtual void OnAbilitySystemUninitialized() {}


	////////////////////////////////////////////////
	// Stance
public:
	virtual bool CanJumpInternal_Implementation() const;

	virtual bool CanCrouch() const override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;


	////////////////////////////////////////////////
	// View
public:
	/**
	 * GetViewRotation
	 *
	 *  現在の ViewState.Rotation を取得する
	 */
	virtual FRotator GetViewRotation() const override;

	/**
	 * GetViewRotationSuperClass
	 *
	 *  親関数の GetViewRotation() を呼び出す
	 * 
	 *  オーバーライドした GetViewRotation() は全く別のデータ取得に置き換えるため
	 *  元々の GetViewRotation() の機能にアクセスるために使用。
	 */
	virtual FRotator GetViewRotationSuperClass() const;


	////////////////////////////////////////////////
	// Jump
public:
	virtual void OnJumped_Implementation() override;

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnJumpedNetworked();
	void Multicast_OnJumpedNetworked_Implementation();

	void OnJumpedNetworked();


	//////////////////////////////////////////
	// Rotation
public:
	/**
	 * FaceRotation
	 *
	 *  この関数の処理は使用しない
	 */
	virtual void FaceRotation(FRotator Rotation, float DeltaTime) override final {}
	

	////////////////////////////////////////////////
	// チーム関係
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


	////////////////////////////////////////////////
	// Pawn の Mesh をアシスト
public:
	//~IBEPawnMeshAssistInterface interface
	void GetMeshes_Implementation(TArray<USkeletalMeshComponent*>& Meshes) const override;
	void GetFPPMeshes_Implementation(TArray<USkeletalMeshComponent*>& Meshes) const override;
	void GetTPPMeshes_Implementation(TArray<USkeletalMeshComponent*>& Meshes) const override;
	void GetFPPFirstMesh_Implementation(USkeletalMeshComponent*& Mesh) const override;
	void GetTPPFirstMesh_Implementation(USkeletalMeshComponent*& Mesh) const override;

	void GetMainAnimInstance_Implementation(UBEAnimInstance*& Instance) const override;
	void GetSubAnimInstances_Implementation(TArray<UAnimInstance*>& Instances) const override;
	void GetTPPAnimInstance_Implementation(UAnimInstance*& Instance) const override;
	void GetFPPAnimInstance_Implementation(UAnimInstance*& Instance) const override;
	//~End of IBEPawnMeshAssistInterface interface


	////////////////////////////////////////////////
	// Utilities
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	ABEPlayerController* GetBEPlayerController() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	ABEPlayerState* GetBEPlayerState() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	UBEPawnBasicComponent* GetBEPawnBasic() const { return CharacterBasic; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	UBECharacterMovementComponent* GetBECharacterMovement() const { return BECharacterMovement; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	UBECharacterAnimInstance* GetBEMainTPPAnimInstance() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	virtual UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
};
