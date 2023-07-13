// Copyright owoDra

#pragma once

#include "ModularCharacter.h"

#include "State/BELocomotionState.h"
#include "State/BEMovementBaseState.h"
#include "State/BERagdollingState.h"
#include "State/BEViewState.h"
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
	virtual void PreRegisterAllComponents() override;
	virtual void PostInitializeComponents() override;

	virtual void Restart() override;
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
	// Movement Base
protected:
	//
	// Character の移動の基盤となる情報
	// 例えば Mantle 中は乗り越え中のオブジェクトがベースとなる
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient)
	FMovementBaseState MovementBase;

private:
	/**
	 * RefreshUsingAbsoluteRotation
	 *
	 *  絶対回転を使用するように更新する
	 * 
	 *  Character の回転を正確に同期できるように Mesh の絶対回転を使用する
	 *  AnimInstance から Mesh の回転を手動で更新することでアニメーションを更新する
	 * 
	 *  これは Character と AnimInstance が 異なる頻度で更新されている場合に必要で
	 *  Character と AnimInstance の回転の同期を助ける
	 * 
	 *  パフォーマンスを節約するため、本当に必要な場合にのみ使用する
	 *  例えば URO が有効な場合、またはリッスン サーバー上の自律プロキシの場合
	 */
	void RefreshUsingAbsoluteRotation() const;

	/**
	 * RefreshVisibilityBasedAnimTickOption
	 *
	 *  Visibility と Tick の設定を更新する
	 */
	void RefreshVisibilityBasedAnimTickOption() const;

	/**
	 * RefreshMovementBase
	 *
	 *  MovementBase を更新する
	 */
	void RefreshMovementBase();

	/**
	 * RefreshAnimInstanceMovement
	 *
	 *  AnimInstance に更新を知らせる
	 */
	void RefreshAnimInstanceMovement();


	////////////////////////////////////////////////
	// Locomotion Mode
protected:
	//
	// Character の現在の移動方式
	// (OnGround, InAir ...)
	// 
	// この Tag は AbilitySystem によって Active な Tag としても適応される。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient)
	FGameplayTag LocomotionMode;

public:
	/**
	 * GetLocomotionMode
	 * 
	 *  現在の Locomotion Mode を取得する
	 */
	const FGameplayTag& GetLocomotionMode() const { return LocomotionMode; }

	/**
	 * SetLocomotionMode
	 *
	 *  現在の Locomotion Mode を設定する
	 */
	void SetLocomotionMode(const FGameplayTag& NewLocomotionMode);

protected:
	/**
	 * OnLocomotionModeChanged
	 *
	 *  Locomotion Mode が変更されたことを知らせる
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character", meta = (DisplayName = "OnLocomotionModeChanged"))
	void K2_OnLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode);
	virtual void OnLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode);


	////////////////////////////////////////////////
	// Desired Rotation Mode
protected:
	//
	// 遷移を望む Character の回転方式
	// (Velocity Direction, View Direction, Aiming ...)
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs|Desired State", Replicated)
	FGameplayTag DesiredRotationMode;

public:
	/**
	 * GetDesiredRotationMode
	 *
	 *  現在の Desired Rotation Mode を取得する
	 */
	const FGameplayTag& GetDesiredRotationMode() const { return DesiredRotationMode; }

	/**
	 * SetDesiredRotationMode
	 *
	 *  現在の Desired Rotation Mode を設定する
	 */
	UFUNCTION(BlueprintCallable, Category = "Character", Meta = (AutoCreateRefTerm = "NewDesiredRotationMode"))
	void SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode);

private:
	/**
	 * Server_SetDesiredRotationMode
	 *
	 *  現在の Desired Rotation Mode をサーバー経由で設定する
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode);
	void Server_SetDesiredRotationMode_Implementation(const FGameplayTag& NewDesiredRotationMode);


	////////////////////////////////////////////////
	// Rotation Mode
protected:
	//
	// Character の現在の回転方式
	// (Velocity Direction, View Direction, Aiming ...)
	// 
	// この Tag は AbilitySystem によって Active な Tag としても適応される。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient)
	FGameplayTag RotationMode;

public:
	/**
	 * RefreshRotationMode
	 *
	 *  現在の Rotation Mode を更新する
	 */
	void RefreshRotationMode();

	/**
	 * GetRotationMode
	 *
	 *  現在の Rotation Mode を取得する
	 */
	const FGameplayTag& GetRotationMode() const { return RotationMode; }

protected:
	/**
	 * SetRotationMode
	 *
	 *  現在の Rotation Mode を設定する
	 */
	void SetRotationMode(const FGameplayTag& NewRotationMode);

	/**
	 * CalculateAllowedRotationMode
	 *
	 *  現在の状態で許可される RotationMode を計算する
	 */
	virtual FGameplayTag CalculateAllowedRotationMode() const;

	/**
	 * OnRotationModeChanged
	 *
	 *  Rotation Mode が変更されたことを知らせる
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character", meta = (DisplayName = "OnRotationModeChanged"))
	void K2_OnRotationModeChanged(const FGameplayTag& PreviousRotationMode);
	virtual void OnRotationModeChanged(const FGameplayTag& PreviousRotationMode);

	
	////////////////////////////////////////////////
	// Desired Stance
protected:
	//
	// 遷移を望む Stance 状態
	// (Standing, Crouching ...)
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs|Desired State", Replicated)
	FGameplayTag DesiredStance;

public:
	/**
	 * GetDesiredStance
	 *
	 *  現在の Desired Stance を取得する
	 */
	const FGameplayTag& GetDesiredStance() const { return DesiredStance; }

	/**
	 * SetDesiredStance
	 *
	 *  現在の Desired Stance を設定する
	 */
	UFUNCTION(BlueprintCallable, Category = "Character", Meta = (AutoCreateRefTerm = "NewDesiredStance"))
	void SetDesiredStance(const FGameplayTag& NewDesiredStance);

protected:
	/**
	 * ApplyDesiredStance
	 *
	 *  現在の Desired Stance を適応する
	 */
	virtual void ApplyDesiredStance();

private:
	/**
	 * Server_SetDesiredStance
	 *
	 *  現在の Desired Stance をサーバー経由で設定する
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredStance(const FGameplayTag& NewDesiredStance);
	void Server_SetDesiredStance_Implementation(const FGameplayTag& NewDesiredStance);


	////////////////////////////////////////////////
	// Stance
protected:
	//
	// Character の現在の Stance 状態
	// (Standing, Crouching ...)
	// 
	// この Tag は AbilitySystem によって Active な Tag としても適応される。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient)
	FGameplayTag Stance;

public:
	virtual bool CanCrouch() const override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	/**
	 * GetStance
	 *
	 *  現在の Stance を取得する
	 */
	const FGameplayTag& GetStance() const { return Stance; }

protected:
	/**
	 * SetStance
	 *
	 *  現在の Stance を設定する
	 */
	void SetStance(const FGameplayTag& NewStance);

	/**
	 * CalculateAllowedStance
	 *
	 *  現在の状態で許可される Stance を計算する
	 */
	virtual FGameplayTag CalculateAllowedStance() const;

	/**
	 * OnStanceChanged
	 *
	 *  Stance が変更されたことを知らせる
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character", meta = (DisplayName = "OnStanceChanged"))
	void K2_OnStanceChanged(const FGameplayTag& PreviousStance);
	virtual void OnStanceChanged(const FGameplayTag& PreviousStance);


	////////////////////////////////////////////////
	// Desired Gait
protected:
	//
	// 遷移を望む Gait 状態
	// (Walk, Run, Sprint ...)
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs|Desired State", Replicated)
	FGameplayTag DesiredGait;

public:
	/**
	 * GetDesiredGait
	 *
	 *  現在の Desired Gait を取得する
	 */
	const FGameplayTag& GetDesiredGait() const { return DesiredGait; }

	/**
	 * SetDesiredGait
	 *
	 *  現在の Desired Gait を設定する
	 */
	UFUNCTION(BlueprintCallable, Category = "Character", Meta = (AutoCreateRefTerm = "NewDesiredGait"))
	void SetDesiredGait(const FGameplayTag& NewDesiredGait);

private:
	/**
	 * Server_SetDesiredGait
	 *
	 *  現在の Desired Gait をサーバー経由で設定する
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredGait(const FGameplayTag& NewDesiredGait);
	void Server_SetDesiredGait_Implementation(const FGameplayTag& NewDesiredGait);


	////////////////////////////////////////////////
	// Gait
protected:
	//
	// Character の現在の Gait 状態
	// (Walk, Run, Sprint ...)
	// 
	// この Tag は AbilitySystem によって Active な Tag としても適応される。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient)
	FGameplayTag Gait;

public:
	/**
	 * RefreshGait
	 *
	 *  現在の Gait を更新する
	 */
	void RefreshGait();

	/**
	 * GetGait
	 *
	 *  現在の Gait を取得する
	 */
	const FGameplayTag& GetGait() const { return Gait; }

private:
	/**
	 * CalculateMaxAllowedGait
	 *
	 *  現在の状態で許可される Gait を計算する
	 */
	virtual FGameplayTag CalculateMaxAllowedGait() const;

	/**
	 * CalculateActualGait
	 *
	 *  現在の移動速度に基づいた実際の Gait を計算する
	 */
	virtual FGameplayTag CalculateActualGait(const FGameplayTag& MaxAllowedGait) const;

	/**
	 * CanSprint
	 *
	 *  現在の Gait を Status.Gait.Sprinting に移行可能かどうか
	 */
	virtual bool CanSprint() const;

protected:
	/**
	 * SetGait
	 *
	 *  現在の Gait を設定する
	 */
	void SetGait(const FGameplayTag& NewGait);

	/**
	 * OnGaitChanged
	 *
	 *  Gait が変更されたことを知らせる
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character", meta = (DisplayName = "OnGaitChanged"))
	void K2_OnGaitChanged(const FGameplayTag& PreviousGait);
	virtual void OnGaitChanged(const FGameplayTag& PreviousGait);


	//////////////////////////////////////////
	// Input
protected:
	//
	// レプリケートされた入力方向
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient, Replicated)
	FVector_NetQuantizeNormal InputDirection;

public:
	/**
	 * RefreshInput
	 *
	 *  現在の InputDirection を更新する
	 */
	void RefreshInput(float DeltaTime);

	/**
	 * GetInputDirection
	 *
	 *  現在の InputDirection を取得する
	 */
	const FVector& GetInputDirection() const { return InputDirection; }

protected:
	/**
	 * SetInputDirection
	 *
	 *  現在の InputDirection を設定する
	 */
	void SetInputDirection(FVector NewInputDirection);


	////////////////////////////////////////////////
	// View
protected:
	//
	// ネットワークでレプリケートされた生の視点の回転情報。
	// 基本的には、ネットワークスムージングを活用した FViewState::Rotation を使用する。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient, ReplicatedUsing = "OnReplicated_ReplicatedViewRotation")
	FRotator ReplicatedViewRotation;

	//
	// Character の視点の状態
	// (回転, 速度 ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient)
	FViewState ViewState;

public:
	/**
	 * GetViewState
	 *
	 *  現在の ViewState を取得する
	 */
	const FViewState& GetViewState() const { return ViewState; }

	/**
	 * GetViewRotation
	 *
	 *  現在の ViewState.Rotation を取得する
	 */
	virtual FRotator GetViewRotation() const override { return ViewState.Rotation; }

	/**
	 * CorrectViewNetworkSmoothing
	 *
	 *  ネットワークスムージングに基づいて現在の ViewState.Rotation を修正する
	 */
	void CorrectViewNetworkSmoothing(const FRotator& NewViewRotation);

	/**
	 * RefreshView
	 *
	 *  現在の ViewState を更新する
	 */
	void RefreshView(float DeltaTime);

private:
	/**
	 * RefreshViewNetworkSmoothing
	 *
	 *  現在の ViewState をネットワークスムージングに基づいて更新する
	 */
	void RefreshViewNetworkSmoothing(float DeltaTime);

	/**
	 * SetReplicatedViewRotation
	 *
	 *  現在の ReplicatedViewRotation を設定する
	 */
	void SetReplicatedViewRotation(const FRotator& NewViewRotation);

	/**
	 * OnReplicated_ReplicatedViewRotation
	 *
	 *  ReplicatedViewRotation がレプリケートされたことを知らせる
	 */
	UFUNCTION()
	void OnReplicated_ReplicatedViewRotation();

	/**
	 * Server_SetReplicatedViewRotation
	 *
	 *  現在の ReplicatedViewRotation をサーバー経由で設定する
	 */
	UFUNCTION(Server, Unreliable)
	void Server_SetReplicatedViewRotation(const FRotator& NewViewRotation);
	void Server_SetReplicatedViewRotation_Implementation(const FRotator& NewViewRotation);


	////////////////////////////////////////////////
	// Locomotion State
protected:
	//
	// Character の移動の状態
	// (入力, 移動速度, 加速度 ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient)
	FLocomotionState LocomotionState;

	//
	// レプリケートされた移動速度の方向角度
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Transient, Replicated, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float DesiredVelocityYawAngle;

public:
	/**
	 * GetLocomotionState
	 *
	 *  現在の Locomotion State を取得する
	 */
	const FLocomotionState& GetLocomotionState() const { return LocomotionState; }

	/**
	 * RefreshLocomotionEarly
	 *
	 *  他の更新より前に実行される移動更新
	 */
	virtual void RefreshLocomotionEarly();

	/**
	 * RefreshLocomotion
	 *
	 *  他の更新の間に実行される移動更新
	 */
	virtual void RefreshLocomotion(float DeltaTime);

	/**
	 * RefreshLocomotionLate
	 *
	 *  他の更新より後に実行される移動更新
	 */
	virtual void RefreshLocomotionLate(float DeltaTime);

private:
	/**
	 * RefreshLocomotionLocationAndRotation
	 *
	 *  移動と回転を更新する
	 */
	virtual void RefreshLocomotionLocationAndRotation();

	/**
	 * SetDesiredVelocityYawAngle
	 *
	 *  現在の DesiredVelocityYawAngle を設定する
	 */
	void SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle);


	//////////////////////////////////////////
	// Jumping
public:
	virtual void Jump() override;
	virtual void OnJumped_Implementation() override;

private:
	/**
	 * MulticastOnJumpedNetworked
	 *
	 *  ジャンプをしたことをネットワークでマルチキャストする
	 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnJumpedNetworked();
	void Multicast_OnJumpedNetworked_Implementation();

	/**
	 * MulticastOnJumpedNetworked
	 *
	 *  ネットワークでジャンプを行ったことを知らせる
	 */
	void OnJumpedNetworked();


	//////////////////////////////////////////
	// Rotation
public:
	/**
	 * FaceRotation
	 *
	 *  この関数の処理は使用しない
	 */
	virtual void FaceRotation(FRotator Rotation, float DeltaTime) override final;

	/**
	 * RefreshGroundedRotation
	 *
	 *  地上にいる時の Character の回転処理を行う
	 */
	void RefreshGroundedRotation(float DeltaTime);

	/**
	 * RefreshInAirRotation
	 *
	 *  空中にいる時の Character の回転処理を行う
	 */
	void RefreshInAirRotation(float DeltaTime);

protected:
	/**
	 * CalculateRotationInterpolationSpeed
	 *
	 *  回転の補間速度を計算する
	 */
	float CalculateRotationInterpolationSpeed() const;

	/**
	 * RefreshGroundedMovingAimingRotation
	 *
	 *  地上で Aiming 状態で移動中の回転を更新する
	 */
	void RefreshGroundedMovingAimingRotation(float DeltaTime);

	/**
	 * RefreshGroundedMovingAimingRotation
	 *
	 *  地上で Aiming 状態で停止中の回転を更新する
	 */
	void RefreshGroundedNotMovingAimingRotation(float DeltaTime);

	/**
	 * RefreshGroundedMovingAimingRotation
	 *
	 *  空中で Aiming 状態の回転を更新する
	 */
	void RefreshInAirAimingRotation(float DeltaTime);

	/**
	 * RefreshRotation
	 *
	 *  回転を更新する
	 */
	void RefreshRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed);

	/**
	 * RefreshRotationExtraSmooth
	 *
	 *  回転をスムーズに更新する
	 */
	void RefreshRotationExtraSmooth(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed);

	/**
	 * RefreshRotationInstant
	 *
	 *  回転を瞬時に更新する
	 */
	void RefreshRotationInstant(float TargetYawAngle, ETeleportType Teleport = ETeleportType::None);

	/**
	 * RefreshTargetYawAngleUsingLocomotionRotation
	 *
	 *  移動回転を用いて視点の角度を更新する
	 */
	void RefreshTargetYawAngleUsingLocomotionRotation();

	/**
	 * RefreshTargetYawAngle
	 *
	 *  視点の角度を更新する
	 */
	void RefreshTargetYawAngle(float TargetYawAngle);

	/**
	 * RefreshViewRelativeTargetYawAngle
	 *
	 *  視点の相対角度を更新する
	 */
	void RefreshViewRelativeTargetYawAngle();
	
private:
	/**
	 * ApplyRotationYawSpeed
	 *
	 *  視点の回転速度を適応する
	 */
	void ApplyRotationYawSpeed(float DeltaTime);
	

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
	USkeletalMeshComponent* GetFPPMesh_Implementation() const override;
	USkeletalMeshComponent* GetTPPMesh_Implementation() const override;

	void GetMainAnimInstances_Implementation(TArray<UBEAnimInstance*>& Instances) const override;
	UBEAnimInstance* GetFPPAnimInstance_Implementation() const override;
	UBEAnimInstance* GetTPPAnimInstance_Implementation() const override;
	//~End of IBEPawnMeshAssistInterface interface


	////////////////////////////////////////////////
	// Utilities
public:
	UFUNCTION(BlueprintCallable, Category = "Character")
	ABEPlayerController* GetBEPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	ABEPlayerState* GetBEPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	FCollisionQueryParams GetIgnoreCharacterParams() const;
};
