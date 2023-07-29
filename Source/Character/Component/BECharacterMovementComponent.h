// Copyright owoDra

#pragma once

#include "GameFramework/CharacterMovementComponent.h"

#include "Character/Movement/State/BEViewState.h"
#include "Character/Movement/State/BELocomotionState.h"
#include "Character/Movement/State/BEMovementBaseState.h"
#include "Character/Movement/BECharacterMovementConfigs.h"

#include "Components/GameFrameworkInitStateInterface.h"

#include "BECharacterMovementComponent.generated.h"

class UBEMovementSet;
class UBECharacterMovementData;
struct GameplayTag;


/**
 * FBECharacterNetworkMoveData
 */
class BECORE_API FBECharacterNetworkMoveData : public FCharacterNetworkMoveData
{
private:
	using Super = FCharacterNetworkMoveData;

public:
	uint8 RotationMode;
	uint8 Stance;
	uint8 MaxAllowedGait;

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
	FBECharacterNetworkMoveData MoveData[3];
};


/**
 * FBESavedMove
 */
class BECORE_API FBESavedMove : public FSavedMove_Character
{
private:
	using Super = FSavedMove_Character;

public:
	uint8 RotationMode;
	uint8 Stance;
	uint8 MaxAllowedGait;

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

	///////////////////////////////////////////////
	// Components
protected:
	FBECharacterNetworkMoveDataContainer MoveDataContainer;

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

	//
	// Character の移動設定
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Configs")
	TObjectPtr<UBECharacterMovementData> MovementData;

	
	///////////////////////////////////////////////
	// Initialize and Uninitialize
protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeWithAbilitySystem(UBEAbilitySystemComponent* InASC);
	void UninitializeFromAbilitySystem();

public:
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface


	///////////////////////////////////////////////
	// Attributes
protected:
	//
	// Character に適応される GravityScale の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attaribute", Transient)
	float GravityScaleScale = 1.0;

	//
	// Character に適応される GroundFriction の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attaribute", Transient)
	float GroundFrictionScale = 1.0;

	//
	// Character に適応される MoveSpeed の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attaribute", Transient)
	float MoveSpeedScale = 1.0;

	//
	// Character に適応される JumpPower の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attaribute", Transient)
	float JumpPowerScale = 1.0;

	//
	// Character に適応される AirControl の倍率
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attaribute", Transient)
	float AirControlScale = 1.0;

protected:
	virtual void HandleGravityScaleScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleGroundFrictionScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMoveSpeedScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleJumpPowerScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleAirControlScaleChanged(const FOnAttributeChangeData& ChangeData);


	////////////////////////////////////////////////
	// Replication
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;


	////////////////////////////////////////////////
	// Movement Base
protected:
	//
	// Character の移動の基盤となる情報
	// 例えば Mantle 中は乗り越え中のオブジェクトがベースとなる
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|MovementBase", Transient)
	FMovementBaseState MovementBase;

public:
	virtual void UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation) override;

	/**
	 * UpdateUsingAbsoluteRotation
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
	void UpdateUsingAbsoluteRotation() const;

	/**
	 * UpdateVisibilityBasedAnimTickOption
	 *
	 *  Visibility と Tick の設定を更新する
	 */
	void UpdateVisibilityBasedAnimTickOption() const;

	/**
	 * UpdateMovementBase
	 *
	 *  MovementBase を更新する
	 */
	void UpdateMovementBase();

	/**
	 * UpdateAnimInstanceMovement
	 *
	 *  AnimInstance に更新を知らせる
	 */
	void UpdateAnimInstanceMovement();


	//////////////////////////////////////////
	// Locomotion Mode
protected:
	//
	// Character の現在の移動方式
	// (OnGround, InAir ...)
	// 
	// Movement Mode が変更されたときにそれに応じて更新される
	// この Tag は AbilitySystem によって Active な Tag としても適応される。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Locomotion Mode", Transient)
	FGameplayTag LocomotionMode;

	//
	// キャラクターの LocomotionMode ごとの設定を定義する
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Locomotion Mode", Transient)
	FBECharacterLocomotionModeConfigs LocomotionModeConfigs;

	//
	// MovementMode の変更を無効にするかどうか
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Locomotion Mode", Transient)
	bool bMovementModeLocked;

public:
	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0) override;

	/**
	 * GetLocomotionMode
	 *
	 *  現在の Locomotion Mode を取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Locomotion Mode")
	const FGameplayTag& GetLocomotionMode() const { return LocomotionMode; }

protected:
	/**
	 * SetMovementModeLocked
	 *
	 *  現在の bMovementModeLocked を設定する
	 */
	void SetMovementModeLocked(bool bNewMovementModeLocked) { bMovementModeLocked = bNewMovementModeLocked; }

	/**
	 * SetLocomotionMode
	 *
	 *  現在の Locomotion Mode を設定する
	 */
	void SetLocomotionMode(const FGameplayTag& NewLocomotionMode);

	/**
	 * OnMovementModeChanged
	 *
	 *  MovementMode が変更されたことを知らせる
	 */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	/**
	 * RefreshLocomotionModeConfigs
	 *
	 *  現在の LocomotionMode に対応する LocomotionModeConfigs に更新する
	 */
	void RefreshLocomotionModeConfigs();


	////////////////////////////////////////////////
	// Desired Rotation Mode
protected:
	//
	// 遷移を望む Character の回転方式
	// (Velocity Direction, View Direction, Aiming ...)
	// 
	// MovementData で定義した LocomotionMode ごとの RotationMode の
	// インデックスに関連している
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs|Rotation Mode", Replicated)
	uint8 DesiredRotationModeIndex;

public:
	/**
	 * GetDesiredRotationModeIndex
	 *
	 *  現在の DesiredRotationModeIndex を取得する
	 */
	uint8 GetDesiredRotationModeIndex() const { return DesiredRotationModeIndex; }

	/**
	 * SetDesiredRotationModeIndex
	 *
	 *  現在の DesiredRotationModeIndex を設定する
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Rotation Mode")
	void SetDesiredRotationModeIndex(uint8 NewDesiredRotationModeIndex);

private:
	/**
	 * Server_SetDesiredRotationModeIndex
	 *
	 *  現在の DesiredRotationModeIndex をサーバー経由で設定する
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredRotationModeIndex(uint8 NewDesiredRotationModeIndex);
	void Server_SetDesiredRotationModeIndex_Implementation(uint8 NewDesiredRotationModeIndex);


	////////////////////////////////////////////////
	// Rotation Mode
protected:
	//
	// Character の現在の回転方式
	// (Velocity Direction, View Direction, Aiming ...)
	// 
	// MovementData で定義した LocomotionMode ごとの RotationMode の
	// インデックスに関連している
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Rotation Mode", Transient)
	uint8 RotationModeIndex;

	//
	// キャラクターの RotationMode ごとの設定を定義する
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Rotation Mode", Transient)
	FBECharacterRotationModeConfigs RotationModeConfigs;

public:
	/**
	 * GetRotationModeIndex
	 *
	 *  現在の RotationModeIndex を取得する
	 */
	uint8 GetRotationModeIndex() const { return RotationModeIndex; }

	/**
	 * GetRotationModeTag
	 *
	 *  現在の RotationModeTag を取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Rotation Mode", meta = (DisplayName = "GetRotationMode"))
	const FGameplayTag& GetRotationModeTag() const { return RotationModeConfigs.RotationModeTag; }

protected:
	/**
	 * SetRotationModeIndex
	 *
	 *  現在の RotationModeIndex を設定する
	 */
	bool SetRotationModeIndex(uint8 NewRotationModeIndex);

	/**
	 * CalculateAllowedRotationModeIndex
	 *
	 *  現在の状態で許可される RotationModeIndex を計算する
	 */
	virtual uint8 CalculateAllowedRotationModeIndex() const;

	/**
	 * UpdateRotationMode
	 *
	 *  現在の Rotation Mode を更新する
	 */
	void UpdateRotationMode(bool bFroceRefreshConfigs = false);

	/**
	 * RefreshRotationModeConfigs
	 *
	 *  現在の RotationMode に対応する RotationModeConfigs に更新する
	 */
	void RefreshRotationModeConfigs();


	////////////////////////////////////////////////
	// Desired Stance
protected:
	//
	// 遷移を望む Stance 状態
	// (Standing, Crouching ...)
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs|Stance", Replicated)
	uint8 DesiredStanceIndex;

public:
	/**
	 * GetDesiredStanceIndex
	 *
	 *  現在の DesiredStanceIndex を取得する
	 */
	uint8 GetDesiredStanceIndex() const { return DesiredStanceIndex; }

	/**
	 * SetDesiredStanceIndex
	 *
	 *  現在の DesiredStanceIndex を設定する
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Stance")
	void SetDesiredStanceIndex(uint8 NewDesiredStanceIndex);

private:
	/**
	 * Server_SetDesiredStanceIndex
	 *
	 *  現在の DesiredStanceIndex をサーバー経由で設定する
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredStanceIndex(uint8 NewDesiredStanceIndex);
	void Server_SetDesiredStanceIndex_Implementation(uint8 NewDesiredStanceIndex);


	////////////////////////////////////////////////
	// Stance
protected:
	//
	// Character の現在の Stance 状態
	// (Standing, Crouching ...)
	// 
	// MovementData で定義した RotationMode ごとの Stance の
	// インデックスに関連している
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Stance", Transient)
	uint8 StanceIndex;
	
	//
	// キャラクターの Stance ごとの設定を定義する
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Stance", Transient)
	FBECharacterStanceConfigs StanceConfigs;

public:
	/**
	 * GetStanceIndex
	 *
	 *  現在の StanceIndex を取得する
	 */
	uint8 GetStanceIndex() const { return StanceIndex; }

	/**
	 * GetStanceTag
	 *
	 *  現在の StanceTag を取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Stance", meta = (DisplayName = "GetStance"))
	const FGameplayTag& GetStanceTag() const { return StanceConfigs.StanceTag; }

	/**
	 * SetStanceIndex
	 *
	 *  現在の StanceIndex を設定する
	 */
	bool SetStanceIndex(uint8 NewStanceIndex);

protected:
	/**
	 * CalculateAllowedStance
	 *
	 *  現在の状態で許可される Stance を計算する
	 */
	virtual uint8 CalculateAllowedStanceIndex() const;

	/**
	 * UpdateStance
	 *
	 *  現在の Stance を更新する
	 */
	void UpdateStance(bool bFroceRefreshConfigs = false);

	/**
	 * RefreshStanceConfigs
	 *
	 *  現在の Stance に対応する StanceConfigs に更新する
	 */
	void RefreshStanceConfigs();


	////////////////////////////////////////////////
	// Desired Gait
protected:
	//
	// 遷移を望む Gait 状態
	// (Walk, Run, Sprint ...)
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs|Gait", Replicated)
	uint8 DesiredGaitIndex;

public:
	/**
	 * GetDesiredGaitIndex
	 *
	 *  現在の DesiredGaitIndex を取得する
	 */
	uint8 GetDesiredGaitIndex() const { return DesiredGaitIndex; }

	/**
	 * SetDesiredGaitIndex
	 *
	 *  現在の DesiredGaitIndex を設定する
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Gait")
	void SetDesiredGaitIndex(uint8 NewDesiredGaitIndex);

private:
	/**
	 * Server_SetDesiredGaitIndex
	 *
	 *  現在の DesiredGaitIndex をサーバー経由で設定する
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredGaitIndex(uint8 NewDesiredGaitIndex);
	void Server_SetDesiredGaitIndex_Implementation(uint8 NewDesiredGaitIndex);


	////////////////////////////////////////////////
	// Gait
protected:
	//
	// Character の最大 Gait 状態
	// (Walk, Run, Sprint ...)
	// 
	// MovementData で定義した Stance ごとの Gait の
	// インデックスに関連している
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	uint8 MaxAllowedGaitIndex;

	//
	// Character の現在の Gait 状態
	// (Walk, Run, Sprint ...)
	// 
	// MovementData で定義した Stance ごとの Gait の
	// インデックスに関連している
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	uint8 GaitIndex;

	//
	// Character の現在の Gait 状態の Tag
	// (Walk, Run, Sprint ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	FGameplayTag GaitTag;

	//
	// Character の現在の Gait 状態での回転補間速度
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	float RotationInterpSpeed = 0.0;

public:
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual float GetMaxSpeed() const override;

	/**
	 * GetGaitIndex
	 *
	 *  現在の GaitIndex を取得する
	 */
	uint8 GetGaitIndex() const { return GaitIndex; }

	/**
	 * GetGaitTag
	 *
	 *  現在の GaitTag を取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Gait", meta = (DisplayName = "GetGait"))
	const FGameplayTag& GetGaitTag() const { return GaitTag; }

protected:
	/**
	 * SetMaxAllowedGaitIndex
	 *
	 *  現在の MaxAllowedGaitIndex を設定する
	 */
	bool SetMaxAllowedGaitIndex(uint8 NewMaxAllowedGaitIndex);

	/**
	 * SetGaitIndex
	 *
	 *  現在の GaitIndex を設定する
	 */
	void SetGaitIndex(uint8 NewGaitIndex);

	/**
	 * CalculateGaitIndexes
	 *
	 *  現在の状態で許可される GaitIndex と移動速度に基づいた実際の GaitIndex を計算する
	 */
	virtual void CalculateGaitIndexes(uint8& OutAllowedGaitIndex, uint8& OutActualGaitIndex);

	/**
	 * UpdateGait
	 *
	 *  現在の Gait を更新する
	 */
	void UpdateGait(bool bFroceRefreshConfigs = false);

	/**
	 * RefreshGaitConfigs
	 *
	 *  現在の Gait に対応する GaitConfigs に更新する
	 */
	void RefreshGaitConfigs();

private:
	/**
	 *  MovementBase の回転速度を取得する
	 */
	bool TryGetMovementBaseRotationSpeed(const FBasedMovementInfo& BasedMovement, FRotator& RotationSpeed);


	//////////////////////////////////////////
	// Locomotion Action
protected:
	//
	// Character の現在のアクション(壁乗り越え, 回避 ...)
	// 基本的に再生された AnimMontage の Notify を通して設定される
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Locomotion Action", Transient)
	FGameplayTag LocomotionAction;

public:
	/**
	 * GetLocomotionAction
	 *
	 *  現在の LocomotionAction を取得する
	 */
	const FGameplayTag& GetLocomotionAction() const { return LocomotionAction; }

	/**
	 * SetLocomotionAction
	 *
	 *  現在の LocomotionAction を設定する
	 */
	void SetLocomotionAction(const FGameplayTag& NewLocomotionAction);


	//////////////////////////////////////////
	// Input
protected:
	//
	// レプリケートされた入力方向
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Input", Transient, Replicated)
	FVector_NetQuantizeNormal InputDirection;

public:
	/**
	 * GetInputDirection
	 *
	 *  現在の InputDirection を取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Input")
	const FVector& GetInputDirection() const { return InputDirection; }

protected:
	virtual FVector ConsumeInputVector() override;

	/**
	 * SetInputDirection
	 *
	 *  現在の InputDirection を設定する
	 */
	void SetInputDirection(FVector NewInputDirection);

	/**
	 * UpdateInput
	 *
	 *  現在の InputDirection を更新する
	 */
	void UpdateInput(float DeltaTime);


	////////////////////////////////////////////////
	// View
protected:
	//
	// ネットワークでレプリケートされた生の視点の回転情報。
	// 基本的には、ネットワークスムージングを活用した FViewState::Rotation を使用する。
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|View", Transient, ReplicatedUsing = "OnReplicated_ReplicatedViewRotation")
	FRotator ReplicatedViewRotation;

	//
	// Character の視点の状態
	// (回転, 速度 ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|View", Transient)
	FViewState ViewState;

public:
	/**
	 * GetViewState
	 *
	 *  現在の ViewState を取得する
	 */
	const FViewState& GetViewState() const { return ViewState; }

protected:
	/**
	 * UpdateView
	 *
	 *  現在の ViewState を更新する
	 */
	void UpdateView(float DeltaTime);

private:
	/**
	 * CorrectViewNetworkSmoothing
	 *
	 *  ネットワークスムージングに基づいて現在の ViewState.Rotation を修正する
	 */
	void CorrectViewNetworkSmoothing(const FRotator& NewViewRotation);

	/**
	 * UpdateViewNetworkSmoothing
	 *
	 *  現在の ViewState をネットワークスムージングに基づいて更新する
	 */
	void UpdateViewNetworkSmoothing(float DeltaTime);

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FLocomotionState LocomotionState;

	//
	// レプリケートされた移動速度の方向角度
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Replicated, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float DesiredVelocityYawAngle;

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

public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	virtual void ComputeFloorDist(
		const FVector& CapsuleLocation,
		float LineDistance,
		float SweepDistance,
		FFindFloorResult& OutFloorResult,
		float SweepRadius,
		const FHitResult* DownwardSweepResult) const override;

	/**
	 * GetLocomotionState
	 *
	 *  現在の Locomotion State を取得する
	 */
	const FLocomotionState& GetLocomotionState() const { return LocomotionState; }

protected:
	virtual bool CanAttemptJump() const override;

	virtual void ControlledCharacterMove(const FVector& InputVector, float DeltaTime) override;
	virtual void PhysWalking(float DeltaTime, int32 Iterations) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void PerformMovement(float DeltaTime) override;
	virtual void SmoothClientPosition(float DeltaTime) override;
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAcceleration) override;

	bool TryConsumePrePenetrationAdjustmentVelocity(FVector& OutVelocity);

	/**
	 * UpdateLocomotionEarly
	 *
	 *  他の更新より前に実行される移動更新
	 */
	virtual void UpdateLocomotionEarly();

	/**
	 * UpdateLocomotion
	 *
	 *  他の更新の間に実行される移動更新
	 */
	virtual void UpdateLocomotion(float DeltaTime);

	/**
	 * UpdateLocomotionLate
	 *
	 *  他の更新より後に実行される移動更新
	 */
	virtual void UpdateLocomotionLate(float DeltaTime);

	/**
	 * UpdateLocomotionLocationAndRotation
	 *
	 *  移動と回転を更新する
	 */
	virtual void UpdateLocomotionLocationAndRotation();

private:
	void SavePenetrationAdjustment(const FHitResult& Hit);
	void ApplyPendingPenetrationAdjustment();

	/**
	 * SetDesiredVelocityYawAngle
	 *
	 *  現在の DesiredVelocityYawAngle を設定する
	 */
	void SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle);


	//////////////////////////////////////////
	// Rotation
public:
	/**
	 * UpdateGroundedRotation
	 *
	 *  地上にいる時の Character の回転処理を行う
	 */
	void UpdateGroundedRotation(float DeltaTime);

	/**
	 * UpdateInAirRotation
	 *
	 *  空中にいる時の Character の回転処理を行う
	 */
	void UpdateInAirRotation(float DeltaTime);

protected:
	/**
	 * CalculateRotationInterpolationSpeed
	 *
	 *  回転の補間速度を計算する
	 */
	float CalculateRotationInterpolationSpeed() const;

	/**
	 * UpdateRotation
	 *
	 *  回転を更新する
	 */
	void UpdateRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed);

	/**
	 * UpdateRotationExtraSmooth
	 *
	 *  回転をスムーズに更新する
	 */
	void UpdateRotationExtraSmooth(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed);

	/**
	 * UpdateRotationInstant
	 *
	 *  回転を瞬時に更新する
	 */
	void UpdateRotationInstant(float TargetYawAngle, ETeleportType Teleport = ETeleportType::None);

	/**
	 * UpdateTargetYawAngleUsingLocomotionRotation
	 *
	 *  移動回転を用いて視点の角度を更新する
	 */
	void UpdateTargetYawAngleUsingLocomotionRotation();

	/**
	 * UpdateTargetYawAngle
	 *
	 *  視点の角度を更新する
	 */
	void UpdateTargetYawAngle(float TargetYawAngle);

	/**
	 * UpdateViewRelativeTargetYawAngle
	 *
	 *  視点の相対角度を更新する
	 */
	void UpdateViewRelativeTargetYawAngle();

private:
	/**
	 * ApplyRotationYawSpeed
	 *
	 *  視点の回転速度を適応する
	 */
	void ApplyRotationYawSpeed(float DeltaTime);


	//////////////////////////////////////////
	// Utilities
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
};
