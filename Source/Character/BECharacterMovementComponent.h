// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#pragma once

#include "GameFramework/CharacterMovementComponent.h"

#include "Engine/HitResult.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathSSE.h"
#include "UObject/UObjectGlobals.h"

#include "BECharacterMovementComponent.generated.h"

class UBEAbilitySystemComponent;
class UBEMovementSet;
class ABECharacter;
class UAnimMontage;
class UObject;
struct FOnAttributeChangeData;
struct FFrame;

//////////////////////////////////////////////

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

//////////////////////////////////////////////

/**
 * EBECustomMovementMode
 *
 *	カスタムムーブメントモード
 */
UENUM(BlueprintType)
enum class EBECustomMovementMode : uint8
{
	None		UMETA(DisplayName = "None"),
	Slide		UMETA(DisplayName = "Slide"),
	Climb		UMETA(DisplayName = "Climb"),
	WallRunR	UMETA(DisplayName = "Wall Run Right"),
	WallRunL	UMETA(DisplayName = "Wall Run Left"),

	MAX			UMETA(Hidden)
};


//////////////////////////////////////////////

/**
 * UBECharacterMovementComponent
 *
 *	このプロジェクトのキャラクタームーブメントコンポーネントクラス
 */
UCLASS(Config = Game)
class BECORE_API UBECharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	//////////////////////////////////////////////
	//	インナークラス
	//////////////////////////////////////////////

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
			FLAG_Aim		= 0x20,
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
		uint8 Saved_bWantsToSprint	: 1;
		uint8 Saved_bWantsToAim		: 1;

		// Other Variables
		uint8 Saved_bClimbCooldown	 : 1;
		uint8 Saved_bWallRunCooldown : 1;
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

	//////////////////////////////////////////////
	//	キャラクタームーブメント
	//////////////////////////////////////////////

	//======================================
	//	初期化
	//======================================
public:
	UBECharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Character Movement: AbilitySystem")
	void InitializeWithAbilitySystem(UBEAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintCallable, Category = "Character Movement: AbilitySystem")
	void UninitializeFromAbilitySystem();

protected:
	virtual void OnUnregister() override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<const UBEMovementSet> MovementSet;


	//==========================================================
	//	移動性能 関係
	// 
	// 移動速度や摩擦度などの移動に関わる性能。
	//==========================================================

	/**
	 * Character Movement (General Settings)
	 */
public:
	// 移動速度全体に影響する速度倍率
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, meta = (DisplayAfter = "GravityScale", ClampMin = "0", UIMin = "0", ForceUnits = "x"))
	float OverallMaxSpeedMultiplier = 1.0;


	/**
	 * Character Movement: Walking
	 */
public:
	// Movement Mode が Walking かつ走り状態の最大移動速度
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (DisplayAfter = "MaxWalkSpeedCrouched", ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MaxWalkSpeedRunning = 550;

	// Movement Mode が Walking かつエイム状態の最大移動速度
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (DisplayAfter = "MaxWalkSpeedCrouched", ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MaxWalkSpeedAiming = 250;


	/**
	 * Character Movement: Sliding
	 */
public:
	// スライディング可能かどうか
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	bool CanSlide = true;

	// スライディング可能な最低速度
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MinSlideSpeed = 300;

	// スライディング開始時の加算速度
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float InitialSlideImpulse = 500;

	// スライディング時の傾斜による加算速度
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float SlidingForce = 980;

	// スライディング時の摩擦係数の乗数
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "x"))
	float SlideGroundFrictionFactor = 0.06;

	// スライディング時の減速度
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float BrakingDecelerationSliding = 512;


	/**
	 * Character Movement: Climb
	 */
public:
	// 壁登り可能かどうか
	UPROPERTY(Category = "Character Movement: Climb", EditAnywhere, BlueprintReadWrite)
	bool CanClimb = true;

	// 壁登り可能な最低落下速度
	UPROPERTY(Category = "Character Movement: Climb", EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "cm/s"))
	float MinClimbFallSpeed = -300;

	// 壁登り時の平行移動最高速度
	UPROPERTY(Category = "Character Movement: Climb", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MaxClimbMoveSpeed = 100;

	// 壁登り時に壁に引き付けられる力
	UPROPERTY(Category = "Character Movement: Climb", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float ClimbWallAttractionForce = 200;

	// 壁登り時に壁に引き付けられる力
	UPROPERTY(Category = "Character Movement: Climb", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float ClimbEndJumpUpForce = 400;

	// 壁登り開始時の初期速度
	UPROPERTY(Category = "Character Movement: Climb", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float ClimbInitialSpeed = 500;

	// 壁登り時の純緑化速度の乗数
	UPROPERTY(Category = "Character Movement: Climb", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float ClimbingGravityScale = 0.25;

	// 壁登り時の減速度
	UPROPERTY(Category = "Character Movement: Climb", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float BrakingDecelerationClimb = 512;


	/**
	 * Character Movement: WallRun
	 */
public:
	// 壁走り可能かどうか
	UPROPERTY(Category = "Character Movement: WallRun", EditAnywhere, BlueprintReadWrite)
	bool CanWallRun = true;

	//壁走り可能な最低落下速度
	UPROPERTY(Category = "Character Movement: WallRun", EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "cm/s"))
	float MinWallRunFallSpeed = -300;

	// 壁走り開始可能な最低平行移動速度
	UPROPERTY(Category = "Character Movement: WallRun", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MinEnterWallRunSpeed = 200;

	// 壁走り継続可能な最低平行移動速度
	UPROPERTY(Category = "Character Movement: WallRun", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MinContinueWallRunSpeed = 50;

	// 壁走り時に壁に引き付けられる力
	UPROPERTY(Category = "Character Movement: WallRun", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float WallRunWallAttractionForce = 200;

	// 壁走り時に壁に引き付けられる力
	UPROPERTY(Category = "Character Movement: WallRun", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float WallRunEndJumpUpForce = 500;

	// 壁走り時に壁に引き付けられる力
	UPROPERTY(Category = "Character Movement: WallRun", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float WallRunEndJumpOffForce = 500;

	// 壁走り時の純緑化速度の乗数
	UPROPERTY(Category = "Character Movement: WallRun", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float WallRunGravityScale = 0.5;

	// 壁走り時の減速度
	UPROPERTY(Category = "Character Movement: WallRun", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float BrakingDecelerationWallRun = 512;


	//==========================================================
	//	移動状態 関係
	// 
	// Crouch, Run, Aim など移動モードに関係して作用すること。
	//==========================================================

	/**
	 * Character Movement: Jumping / Falling
	 */
public:
	virtual bool CanAttemptJump() const override;

	/**
	 * Character Movement: Crouch
	 */
public:
	virtual void Crouch(bool bClientSimulation = false) override;


	/**
	 * Character Movement: Running
	 */
public:
	uint8 bWantsToRun : 1;

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool IsRunning() const;

	virtual bool CanRunInCurrentState() const;

	virtual void Run(bool bClientSimulation);
	virtual void UnRun(bool bClientSimulation);


	/**
	 * Character Movement: Aiming
	 */
public:
	uint8 bWantsToAim : 1;

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool IsAiming() const;

	virtual bool CanAimInCurrentState() const;

	virtual void Aim(bool bClientSimulation);
	virtual void UnAim(bool bClientSimulation);


	//==========================================================
	//	移動情報 関係
	// 
	// 移動中の地面や加速度などの情報。
	//==========================================================

	/**
	 * Character Info
	 */
public:
	/** Get the Character that owns UpdatedComponent. */
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	ABECharacter* GetBECharacterOwner() const { return Cast<ABECharacter>(CharacterOwner); }


	/**
	 * Ground Info
	 */
public:
	// 現在の地面の情報を返します。古い場合は、これを呼び出すと地面情報が更新されます。
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	const FBECharacterGroundInfo& GetGroundInfo();

protected:
	UPROPERTY(Transient)
	FBECharacterGroundInfo CachedGroundInfo;


	/**
	 * Movement Info
	 */
public:
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;

	virtual float GetMaxSpeed() const override;

	virtual float GetMaxBrakingDeceleration() const override;

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	virtual float GetSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	virtual float GetSpeed2D() const;

	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	virtual EBECustomMovementMode GetCustomMovementMode() const { return EBECustomMovementMode(CustomMovementMode); }


	/**
	 * Replication Info
	 */
public:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	void SetReplicatedAcceleration(const FVector& InAcceleration);

	virtual void SimulateMovement(float DeltaTime) override;

protected:
	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;


	//==========================================================
	//	移動処理 関係
	// 
	// 移動に関する処理。
	//==========================================================

	/**
	 * General
	 */
public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual bool IsMovingOnGround() const override;

	virtual bool IsMovingInAir() const;

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations);


	/**
	 * Character Movement: Slide
	 */
public:
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool IsSliding() const;

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool TrySlide();

protected:
	virtual void PhysSlide(float deltaTime, int32 Iterations);
	virtual bool CanSlideInCurrentState() const;
	virtual void OnStartSlide();
	virtual void OnEndSlide();


	/**
	 * Character Movement: Climb
	 */
public:
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool IsClimbing() const;

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool TryClimb();

protected:
	virtual void PhysClimb(float deltaTime, int32 Iterations);
	virtual bool CanClimbInCurrentState() const;
	virtual void OnStartClimb();
	virtual void OnEndClimb();

protected:
	uint8 bClimbCooldown : 1;


	/**
	 * Character Movement: WallRun
	 */
public:
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool IsWallRunning() const;

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool IsWallRunningRight() const;

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool IsWallRunningLeft() const;

	UFUNCTION(BlueprintCallable, Category = "Character Movement")
	virtual bool TryWallRun();

protected:
	virtual void PhysWallRun(float deltaTime, int32 Iterations);
	virtual bool CanWallRunInCurrentState() const;
	virtual void OnStartWallRun();
	virtual void OnEndWallRun();

protected:
	uint8 bWallRunCooldown : 1;


	//======================================
	//	アトリビュート 関係
	//======================================
protected:
	virtual void HandleGravityScaleChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleGroundFrictionChanged(const FOnAttributeChangeData& ChangeData);

	virtual void HandleOverallSpeedMultiplierChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleWalkSpeedChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleWalkSpeedCrouchedChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleWalkSpeedRunningChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleWalkSpeedAimingChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleSwimSpeedChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleFlySpeedChanged(const FOnAttributeChangeData& ChangeData);

	virtual void HandleJumpPowerChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleAirControlChanged(const FOnAttributeChangeData& ChangeData);
};
