// Copyright owoDra

#pragma once

#include "GameFramework/CharacterMovementComponent.h"

#include "Components/GameFrameworkInitStateInterface.h"

#include "BECharacterMovementComponent.generated.h"

class UBEMovementSet;
class UBECharacterMovementConfigs;
struct GameplayTag;
struct FMovementGaitConfigs;


/**
 * FBECharacterNetworkMoveData
 */
class BECORE_API FBECharacterNetworkMoveData : public FCharacterNetworkMoveData
{
private:
	using Super = FCharacterNetworkMoveData;

public:
	FGameplayTag RotationMode;
	FGameplayTag Stance;
	FGameplayTag MaxAllowedGait;

public:
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& Movement, FArchive& Archive, UPackageMap* Map, ENetworkMoveType MoveType) override;
};


/**
 * FBECharacterNetworkMoveDataContainer
 */
class BECORE_API FBECharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
public:
	FBECharacterNetworkMoveDataContainer();

public:
	FBECharacterNetworkMoveDataContainer MoveData[3];
};


/**
 * FBESavedMove
 */
class BECORE_API FBESavedMove : public FSavedMove_Character
{
private:
	using Super = FSavedMove_Character;

public:
	FGameplayTag RotationMode;
	FGameplayTag Stance;
	FGameplayTag MaxAllowedGait;

public:
	virtual void Clear() override;

	virtual void SetMoveFor(ACharacter* Character, float NewDeltaTime, const FVector& NewAcceleration,
		FNetworkPredictionData_Client_Character& PredictionData) override;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const override;

	virtual void CombineWith(const FSavedMove_Character* PreviousMove, ACharacter* Character,
		APlayerController* Player, const FVector& PreviousStartLocation) override;

	virtual void PrepMoveFor(ACharacter* Character) override;
};


/**
 * FBENetworkPredictionData
 */
class BECORE_API FBENetworkPredictionData : public FNetworkPredictionData_Client_Character
{
private:
	using Super = FNetworkPredictionData_Client_Character;

public:
	explicit FBENetworkPredictionData(const UCharacterMovementComponent& Movement);

	virtual FSavedMovePtr AllocateNewMove() override;
};


/**
 * UBECharacterMovementComponent
 *
 *	Character の Movement 関係の処理を担当するコンポーネント
 */
UCLASS()
class BECORE_API UBECharacterMovementComponent 
	: public UCharacterMovementComponent
	, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	friend FBESavedMove;
	
public:
	UBECharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	//
	// このコンポーネントを実装する際の FeatureName
	// 
	static const FName NAME_ActorFeatureName;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif


protected:
	FBECharacterNetworkMoveDataContainer MoveDataContainer;

	//
	// Character の移動設定
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Configs")
	TObjectPtr<UBECharacterMovementConfigs> MovementConfigs;

public:
	//
	// MovementMode をロックするか
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	bool bMovementModeLocked;

	//
	// ひとつ前の更新の ControlRotation
	// Locally Controlled の Character のみ更新される。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotator PreviousControlRotation;

	//
	// めり込み時の位置補正
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PendingPenetrationAdjustment;

	//
	// めり込み時の位置補正前の速度
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PrePenetrationAdjustmentVelocity;

	//
	// めり込み時の位置補正前の速度が有効かどうか
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	bool bPrePenetrationAdjustmentVelocityValid;


protected:
	//
	// Character に適応される GravityScale の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float GravityScaleScale = 1.0;

	//
	// Character に適応される GroundFriction の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float GroundFrictionScale = 1.0;

	//
	// Character に適応される MoveSpeed の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float MoveSpeedScale = 1.0;

	//
	// Character に適応される JumpPower の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float JumpPowerScale = 1.0;

	//
	// Character に適応される AirControl の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	float AirControlScale = 1.0;


protected:
	//
	// Owning Character が登録されている AbilitySystem
	//
	UPROPERTY(Transient)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	//
	// 移動に関係する Attribute Set
	//
	UPROPERTY(Transient)
	TObjectPtr<const UBEMovementSet> MovementSet;


public:
	/**
	 *	このコンポーネントを所有する BECharacter を取得する
	 */
	ABECharacter* GetBECharacterOwner() const { return Cast<ABECharacter>(CharacterOwner); }

	/**
	 *	Character からこのコンポーネントを取得する
	 */
	UFUNCTION(BlueprintPure, Category = "Character")
	static UBECharacterMovementComponent* FindCharacterMovementComponent(const ABECharacter* Character);


protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeWithAbilitySystem(UBEAbilitySystemComponent* InASC);
	void UninitializeFromAbilitySystem();
	void InitializeGameplayTags();

	
public:
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface


protected:
	virtual void HandleGravityScaleScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleGroundFrictionScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMoveSpeedScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleJumpPowerScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleAirControlScaleChanged(const FOnAttributeChangeData& ChangeData);


public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0) override;
	virtual void UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation) override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual float GetMaxSpeed() const override;

	virtual void PhysicsRotation(float DeltaTime) override;

	virtual void ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult,
		float SweepRadius, const FHitResult* DownwardSweepResult) const override;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	virtual void ControlledCharacterMove(const FVector& InputVector, float DeltaTime) override;
	virtual FVector ConsumeInputVector() override;

	virtual void PhysWalking(float DeltaTime, int32 Iterations) override;
	virtual void PhysNavWalking(float DeltaTime, int32 Iterations) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	virtual void PerformMovement(float DeltaTime) override;

	virtual void SmoothClientPosition(float DeltaTime) override;
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAcceleration) override;

private:
	bool TryGetMovementBaseRotationSpeed(const FBasedMovementInfo& BasedMovement, FRotator& RotationSpeed);
	void SavePenetrationAdjustment(const FHitResult& Hit);
	void ApplyPendingPenetrationAdjustment();

public:
	void SetMovementModeLocked(bool bNewMovementModeLocked);
	bool TryConsumePrePenetrationAdjustmentVelocity(FVector& OutVelocity);


	//////////////////////////////////////////
	// Locomotion Mode
protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;


	//////////////////////////////////////////
	// Rotation Mode
protected:
	//
	// Character の現在の回転方式
	// (Velocity Direction, View Direction, Aiming ...)
	// 
	// この Tag は AbilitySystem によって Active な Tag としても適応される。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag RotationMode;

public:
	/**
	 * SetRotationMode
	 *
	 *  現在の Rotation Mode を設定する
	 */
	void SetRotationMode(const FGameplayTag& NewRotationMode);


	//////////////////////////////////////////
	// Stance
protected:
	//
	// Character の現在の Stance 状態
	// (Standing, Crouching ...)
	// 
	// この Tag は AbilitySystem によって Active な Tag としても適応される。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Stance;

public:
	/**
	 * SetStance
	 *
	 *  現在の Stance を設定する
	 */
	void SetStance(const FGameplayTag& NewStance);


	//////////////////////////////////////////
	// Gait
protected:
	//
	// Character の現在の状態の Gait 設定
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FMovementGaitConfigs GaitConfigs;

	//
	// Character の最大 Gait 状態
	// (Walk, Run, Sprint ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag MaxAllowedGait;

public:
	/**
	 * SetMaxAllowedGait
	 *
	 *  現在の MaxAllowedGait を設定する
	 */
	void SetMaxAllowedGait(const FGameplayTag& NewMaxAllowedGait);

	/**
	 * CalculateGaitAmount
	 *
	 *  現在の移動速度に基づいた Gait の重み値
	 */
	virtual float CalculateGaitAmount() const;

	/**
	 * RefreshGaitConfigs
	 * 
	 *  現在の状態における Gait 設定を更新する
	 *  この関数は Stance または RotationMode が更新されたときに呼び出される必要がある
	 */
	virtual void RefreshGaitConfigs();

	/**
	 * RefreshMaxWalkSpeed
	 *
	 *  現在の Gait における 移動速度を更新する
	 *  MaxAllowedGait または GaitConfigs が更新されたときに呼び出される必要がある
	 */
	virtual void RefreshMaxWalkSpeed();



	//////////////////////////////////////////
	// Rotation
public:
	void CharacterMovement_OnPhysicsRotation(float DeltaTime);

private:
	void RefreshGroundedRotation(float DeltaTime);

protected:
	virtual bool RefreshCustomGroundedMovingRotation(float DeltaTime);

	virtual bool RefreshCustomGroundedNotMovingRotation(float DeltaTime);

	void RefreshGroundedMovingAimingRotation(float DeltaTime);

	void RefreshGroundedNotMovingAimingRotation(float DeltaTime);

	float CalculateRotationInterpolationSpeed() const;

private:
	void ApplyRotationYawSpeed(float DeltaTime);

	void RefreshInAirRotation(float DeltaTime);

protected:
	virtual bool RefreshCustomInAirRotation(float DeltaTime);

	void RefreshInAirAimingRotation(float DeltaTime);

	void RefreshRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed);

	void RefreshRotationExtraSmooth(float TargetYawAngle, float DeltaTime,
		float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed);

	void RefreshRotationInstant(float TargetYawAngle, ETeleportType Teleport = ETeleportType::None);

	void RefreshTargetYawAngleUsingLocomotionRotation();

	void RefreshTargetYawAngle(float TargetYawAngle);

	void RefreshViewRelativeTargetYawAngle();
};
