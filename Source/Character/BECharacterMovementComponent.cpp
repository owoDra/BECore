// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BECharacterMovementComponent.h"

#include "Ability/BEAbilitySystemComponent.h"
#include "Ability/Attributes/BEMovementSet.h"
#include "Character/BECharacter.h"
#include "BELogChannels.h"
#include "BEGameplayTags.h"

#include "AbilitySystemGlobals.h"
#include "CharacterMovementComponentAsync.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Containers/EnumAsByte.h"
#include "CoreGlobals.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "HAL/IConsoleManager.h"
#include "Math/Vector.h"
#include "Misc/AssertionMacros.h"
#include "NativeGameplayTags.h"
#include "Stats/Stats2.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
#include "Curves/CurveVector.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterMovementComponent)

//////////////////////////////////////////////

namespace BECharacter
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("BECharacter.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
};

//////////////////////////////////////////////
//	インナークラス

#pragma region Saved Move

void UBECharacterMovementComponent::FSavedMove_BECharacter::Clear()
{
	Super::Clear();

	Saved_bWantsToRun		= false;
	Saved_bWantsToAim		= false;

	Saved_bClimbCooldown	= false;
	Saved_bWallRunCooldown	= false;
}

uint8 UBECharacterMovementComponent::FSavedMove_BECharacter::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToRun)
	{
		Result |= FLAG_Run;
	}

	if (Saved_bWantsToAim)
	{
		Result |= FLAG_Aim;
	}

	return Result;
}

bool UBECharacterMovementComponent::FSavedMove_BECharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	const FSavedMove_BECharacter* NewSavedMove = static_cast<FSavedMove_BECharacter*>(NewMove.Get());

	if (Saved_bWantsToRun != NewSavedMove->Saved_bWantsToRun)
	{
		return false;
	}

	if (Saved_bWantsToAim != NewSavedMove->Saved_bWantsToAim)
	{
		return false;
	}

	if (Saved_bClimbCooldown != NewSavedMove->Saved_bClimbCooldown)
	{
		return false;
	}

	if (Saved_bWallRunCooldown != NewSavedMove->Saved_bWallRunCooldown)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UBECharacterMovementComponent::FSavedMove_BECharacter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UBECharacterMovementComponent* CharacterMovement = Cast<UBECharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		Saved_bWantsToRun		= CharacterMovement->bWantsToRun;
		Saved_bWantsToAim		= CharacterMovement->bWantsToAim;

		Saved_bClimbCooldown	= CharacterMovement->bClimbCooldown;
		Saved_bWallRunCooldown	= CharacterMovement->bWallRunCooldown;
	}
}

void UBECharacterMovementComponent::FSavedMove_BECharacter::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UBECharacterMovementComponent* CharacterMovement = Cast<UBECharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->bWantsToRun = Saved_bWantsToRun;
		CharacterMovement->bWantsToAim = Saved_bWantsToAim;

		CharacterMovement->bClimbCooldown	= Saved_bClimbCooldown;
		CharacterMovement->bWallRunCooldown = Saved_bWallRunCooldown;
	}
}

#pragma endregion

#pragma region Client Network Prediction Data

UBECharacterMovementComponent::FNetworkPredictionData_Client_BECharacter::FNetworkPredictionData_Client_BECharacter(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UBECharacterMovementComponent::FNetworkPredictionData_Client_BECharacter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_BECharacter());
}

#pragma endregion


//////////////////////////////////////////////
//	キャラクタームーブメント

#pragma region CMC Initialization

UBECharacterMovementComponent::UBECharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = nullptr;
	MovementSet = nullptr;

	// 基本設定
	GravityScale = 1.0f;
	MaxAcceleration = 2400.0f;
	BrakingFrictionFactor = 1.0f;
	BrakingFriction = 6.0f;
	GroundFriction = 8.0f;
	RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	bUseControllerDesiredRotation = false;
	bOrientRotationToMovement = false;
	bAllowPhysicsRotationDuringAnimRootMotion = false;
	bCanWalkOffLedgesWhenCrouching = true;
	GetNavAgentPropertiesRef().bCanCrouch = true;
	SetCrouchedHalfHeight(65.0f);

	// Character Movement: Walking 設定
	MaxWalkSpeed = 250.0f;
	MaxWalkSpeedCrouched = 200.0f;
}

void UBECharacterMovementComponent::InitializeWithAbilitySystem(UBEAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogBE, Error, TEXT("BECharacterMovementComponent: Character movement component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogBE, Error, TEXT("BECharacterMovementComponent: Cannot initialize Character movement component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	MovementSet = AbilitySystemComponent->GetSet<UBEMovementSet>();
	if (!MovementSet)
	{
		UE_LOG(LogBE, Error, TEXT("BECharacterMovementComponent: Cannot initialize Character movement component for owner [%s] with NULL movement set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// コールバックを登録
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetGravityScaleAttribute()).AddUObject(this, &ThisClass::HandleGravityScaleChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetGroundFrictionAttribute()).AddUObject(this, &ThisClass::HandleGroundFrictionChanged);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetOverallSpeedMultiplierAttribute()).AddUObject(this, &ThisClass::HandleOverallSpeedMultiplierChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetWalkSpeedAttribute()).AddUObject(this, &ThisClass::HandleWalkSpeedChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetWalkSpeedCrouchedAttribute()).AddUObject(this, &ThisClass::HandleWalkSpeedCrouchedChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetWalkSpeedRunningAttribute()).AddUObject(this, &ThisClass::HandleWalkSpeedRunningChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetWalkSpeedAimingAttribute()).AddUObject(this, &ThisClass::HandleWalkSpeedAimingChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetSwimSpeedAttribute()).AddUObject(this, &ThisClass::HandleSwimSpeedChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetFlySpeedAttribute()).AddUObject(this, &ThisClass::HandleFlySpeedChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetJumpPowerAttribute()).AddUObject(this, &ThisClass::HandleJumpPowerChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetAirControlAttribute()).AddUObject(this, &ThisClass::HandleAirControlChanged);

	// アトリビュートの初期値を設定
	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetGravityScaleAttribute(), GravityScale);
	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetGroundFrictionAttribute(), GroundFriction);

	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetOverallSpeedMultiplierAttribute(), OverallMaxSpeedMultiplier);
	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetWalkSpeedAttribute(), MaxWalkSpeed);
	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetWalkSpeedCrouchedAttribute(), MaxWalkSpeedCrouched);
	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetWalkSpeedRunningAttribute(), MaxWalkSpeedRunning);
	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetWalkSpeedAimingAttribute(), MaxWalkSpeedAiming);
	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetSwimSpeedAttribute(), MaxSwimSpeed);
	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetFlySpeedAttribute(), MaxFlySpeed);

	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetJumpPowerAttribute(), JumpZVelocity);
	AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetAirControlAttribute(), AirControl);
}

void UBECharacterMovementComponent::UninitializeFromAbilitySystem()
{
	MovementSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void UBECharacterMovementComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

#pragma endregion

#pragma region CMC Movement States

bool UBECharacterMovementComponent::CanAttemptJump() const
{
	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		if (UBEAbilitySystemComponent* BEASC = BEChara->GetBEAbilitySystemComponent())
		{
			if (BEASC->HasMatchingGameplayTag(TAG_Status_JumpBlocked))
			{
				return false;
			}
		}
	}

	return IsJumpAllowed() && IsMovingOnGround();
}


void UBECharacterMovementComponent::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);

	TrySlide();
}


bool UBECharacterMovementComponent::IsRunning() const
{
	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		return BEChara->bIsRunning;
	}

	return false;
}

bool UBECharacterMovementComponent::CanRunInCurrentState() const
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Status_RunBlocked))
		{
			return false;
		}
	}

	return !IsAiming() && (IsFalling() || IsMovingOnGround());
}

void UBECharacterMovementComponent::Run(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		if (!bClientSimulation)
		{
			BEChara->bIsRunning = true;
		}

		BEChara->OnStartRun();
	}
}

void UBECharacterMovementComponent::UnRun(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		if (!bClientSimulation)
		{
			BEChara->bIsRunning = false;
		}

		BEChara->OnEndRun();
	}
}


bool UBECharacterMovementComponent::IsAiming() const
{
	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		return BEChara->bIsAiming;
	}

	return false;
}

bool UBECharacterMovementComponent::CanAimInCurrentState() const
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Status_WeaponAimBlocked))
		{
			return false;
		}
	}

	return (IsFalling() || IsMovingOnGround() || IsSliding());
}

void UBECharacterMovementComponent::Aim(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		if (!bClientSimulation)
		{
			BEChara->bIsAiming = true;
		}

		BEChara->OnStartAim();
	}
}

void UBECharacterMovementComponent::UnAim(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		if (!bClientSimulation)
		{
			BEChara->bIsAiming = false;
		}

		BEChara->OnEndAim();
	}
}

#pragma endregion

#pragma region CMC Movement Info

const FBECharacterGroundInfo& UBECharacterMovementComponent::GetGroundInfo()
{
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}

	if (MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - BECharacter::GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BECharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = BECharacter::GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;

	return CachedGroundInfo;
}


FRotator UBECharacterMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Status_MovementStopped))
		{
			return FRotator(0, 0, 0);
		}
	}

	return Super::GetDeltaRotation(DeltaTime);
}

float UBECharacterMovementComponent::GetMaxSpeed() const
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Status_MovementStopped))
		{
			return 0.0f;
		}
	}

	EMovementMode LocMoveMode = MovementMode;
	float SuggestMaxSpeed = 0.0;

	if ((LocMoveMode == MOVE_Walking) || (LocMoveMode == MOVE_NavWalking))
	{
		if (IsAiming())
		{
			if (IsCrouching())
			{
				return FMath::Min(MaxWalkSpeedCrouched, MaxWalkSpeedAiming);
			}
			else
			{
				return MaxWalkSpeedAiming;
			}
		}
		else
		{
			if (IsCrouching())
			{
				SuggestMaxSpeed = MaxWalkSpeedCrouched;
			}
			else
			{
				if (IsRunning())
				{
					SuggestMaxSpeed = MaxWalkSpeedRunning;
				}
				else
				{
					SuggestMaxSpeed = MaxWalkSpeed;
				}
			}
		}
	}
	else if (LocMoveMode == MOVE_Falling)
	{
		if (IsRunning())
		{
			SuggestMaxSpeed = MaxWalkSpeedRunning;
		}
		else
		{
			SuggestMaxSpeed = MaxWalkSpeed;
		}
	}
	else if (LocMoveMode == MOVE_Swimming)
	{
		SuggestMaxSpeed = MaxSwimSpeed;
	}
	else if (LocMoveMode == MOVE_Flying)
	{
		SuggestMaxSpeed = MaxFlySpeed;
	}
	else if (LocMoveMode == MOVE_Custom)
	{
		EBECustomMovementMode LocCustomMoveMode = GetCustomMovementMode();

		if (LocCustomMoveMode == EBECustomMovementMode::Climb)
		{
			SuggestMaxSpeed = MaxClimbMoveSpeed;
		}
		else
		{
			SuggestMaxSpeed = MaxCustomMovementSpeed;
		}
	}

	return SuggestMaxSpeed * OverallMaxSpeedMultiplier;
}

float UBECharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode == MOVE_Custom)
	{
		EBECustomMovementMode LocCustomMoveMode = GetCustomMovementMode();

		if (LocCustomMoveMode == EBECustomMovementMode::Slide)
		{
			return BrakingDecelerationSliding;
		}
		else if (LocCustomMoveMode == EBECustomMovementMode::Climb)
		{
			return BrakingDecelerationClimb;
		}
	}

	return Super::GetMaxBrakingDeceleration();
}

float UBECharacterMovementComponent::GetSpeed() const
{
	return Velocity.Size();
}

float UBECharacterMovementComponent::GetSpeed2D() const
{
	return Velocity.Size2D();
}


void UBECharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToRun		= (Flags & FSavedMove_BECharacter::FLAG_Run) != 0;

	bWantsToAim		= (Flags & FSavedMove_BECharacter::FLAG_Aim) != 0;
}

FNetworkPredictionData_Client* UBECharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	if (!ClientPredictionData)
	{
		UBECharacterMovementComponent* MutableThis = const_cast<UBECharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_BECharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UBECharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAcceleration;
}

void UBECharacterMovementComponent::SimulateMovement(float DeltaTime)
{
	if (bHasReplicatedAcceleration)
	{
		// Preserve our replicated acceleration
		const FVector OriginalAcceleration = Acceleration;
		Super::SimulateMovement(DeltaTime);
		Acceleration = OriginalAcceleration;
	}
	else
	{
		Super::SimulateMovement(DeltaTime);
	}
}

#pragma endregion

#pragma region CMC Movement Process

void UBECharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		const bool bIsCrouching = IsCrouching();
		if (bIsCrouching && (!bWantsToCrouch || !CanCrouchInCurrentState()))
		{
			UnCrouch(false);
		}
		else if (!bIsCrouching && bWantsToCrouch && CanCrouchInCurrentState())
		{
			Crouch(false);
		}

		const bool bIsRunning = IsRunning();
		if (bIsRunning && (!bWantsToRun || !CanRunInCurrentState()))
		{
			UnRun(false);
		}
		else if (!bIsRunning && bWantsToRun && CanRunInCurrentState())
		{
			Run(false);
		}

		const bool bIsAiming = IsAiming();
		if (bIsAiming && (!bWantsToAim || !CanAimInCurrentState()))
		{
			UnAim(false);
		}
		else if (!bIsAiming && bWantsToAim && CanAimInCurrentState())
		{
			Aim(false);
		}
	}

	TryClimb();
	TryWallRun();
}

bool UBECharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsSliding();
}

bool UBECharacterMovementComponent::IsMovingInAir() const
{
	return (IsFalling() || IsClimbing() || IsFlying() || IsWallRunning());
}

void UBECharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	bCrouchMaintainsBaseLocation = true;

	if (MovementMode == MOVE_Custom)
	{
		if (GetCustomMovementMode() == EBECustomMovementMode::Slide)
		{
			OnStartSlide();
		}
		else if (GetCustomMovementMode() == EBECustomMovementMode::Climb)
		{
			OnStartClimb();
		}
		else if ((GetCustomMovementMode() == EBECustomMovementMode::WallRunL) || (GetCustomMovementMode() == EBECustomMovementMode::WallRunR))
		{
			OnStartWallRun();
		}
	}
	else if (PreviousMovementMode == MOVE_Custom)
	{
		if (PreviousCustomMode == (uint8)EBECustomMovementMode::Slide)
		{
			OnEndSlide();
		}
		else if (PreviousCustomMode == (uint8)EBECustomMovementMode::Climb)
		{
			OnEndClimb();
		}
		else if ((PreviousCustomMode == (uint8)EBECustomMovementMode::WallRunL) || (PreviousCustomMode == (uint8)EBECustomMovementMode::WallRunR))
		{
			OnEndWallRun();
		}
	}
	else if (MovementMode == MOVE_Walking)
	{
		if (PreviousMovementMode == MOVE_Falling)
		{
			bClimbCooldown = false;
			bWallRunCooldown = false;
			CharacterOwner->StopJumping();
			TrySlide();
		}
	}
}

void UBECharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	EBECustomMovementMode LocCustomMoveMode = GetCustomMovementMode();

	if (LocCustomMoveMode == EBECustomMovementMode::Slide)
	{
		PhysSlide(deltaTime, Iterations);
	}
	else if (LocCustomMoveMode == EBECustomMovementMode::Climb)
	{
		PhysClimb(deltaTime, Iterations);
	}
	else if ((LocCustomMoveMode == EBECustomMovementMode::WallRunL) || (LocCustomMoveMode == EBECustomMovementMode::WallRunR))
	{
		PhysWallRun(deltaTime, Iterations);
	}
}


bool UBECharacterMovementComponent::IsSliding() const
{
	return (GetCustomMovementMode() == EBECustomMovementMode::Slide);
}

void UBECharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CanSlideInCurrentState())
	{
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	bJustTeleported			= false;
	bool bCheckedFall		= false;
	bool bTriedLedgeMove	= false;
	float RemainingTime		= deltaTime;

	// Perform the move
	while ((RemainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float TimeTick = GetSimulationTimeStep(RemainingTime, Iterations);
		RemainingTime -= TimeTick;

		MaintainHorizontalGroundVelocity();

		UPrimitiveComponent* const OldBase			= GetMovementBase();
		const FVector OldVelocity					= Velocity;
		const FVector PreviousBaseLocation			= ((OldBase != NULL) ? (OldBase->GetComponentLocation()) : (FVector::ZeroVector));
		const FVector OldLocation					= UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor				= CurrentFloor;

		FVector SlopeForce	= CurrentFloor.HitResult.Normal;
		SlopeForce.Z		= 0.f;
		Velocity			= Velocity + (SlopeForce * SlidingForce * TimeTick);
		ApplyVelocityBraking(TimeTick, GroundFriction * SlideGroundFrictionFactor, GetMaxBrakingDeceleration());

		const FVector MoveVelocity	= Velocity;
		const FVector Delta			= TimeTick * MoveVelocity;
		const bool bZeroDelta		= Delta.IsNearlyZero();
		const bool bFloorWalkable	= CurrentFloor.IsWalkableFloor();

		FStepDownResult StepDownResult;

		if (bZeroDelta)
		{
			RemainingTime = 0.f;
		}
		else
		{
			MoveAlongFloor(MoveVelocity, TimeTick, &StepDownResult);

			if (IsFalling())
			{
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					RemainingTime += TimeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(RemainingTime, Iterations);
				return;
			}
			else if (IsSwimming())
			{
				StartSwimming(OldLocation, OldVelocity, TimeTick, RemainingTime, Iterations);
				return;
			}
		}

		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		const bool bCheckLedges = !CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			const FVector GravDir = FVector(0.f, 0.f, -1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if (!NewDelta.IsZero())
			{
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				bTriedLedgeMove = true;

				Velocity = NewDelta / TimeTick;
				RemainingTime += TimeTick;
				continue;
			}
			else
			{
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, RemainingTime, TimeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				RemainingTime = 0.f;
				break;
			}
		}
		else
		{
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, TimeTick);
					if (IsMovingOnGround())
					{
						StartFalling(Iterations, RemainingTime, TimeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && RemainingTime <= 0.f)
			{
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			if (IsSwimming())
			{
				StartSwimming(OldLocation, Velocity, TimeTick, RemainingTime, Iterations);
				return;
			}

			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, RemainingTime, TimeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;
			}
		}

		if (IsMovingOnGround() && bFloorWalkable)
		{
			if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && TimeTick >= MIN_TICK_TIME)
			{
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / TimeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			RemainingTime = 0.f;
			break;
		}
	}
}

bool UBECharacterMovementComponent::CanSlideInCurrentState() const
{
	return (CanSlide && IsMovingOnGround() && bWantsToCrouch && bWantsToRun && (GetSpeed() > MinSlideSpeed));
}

bool UBECharacterMovementComponent::TrySlide()
{
	if (!IsSliding() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy))
	{
		if (CanSlideInCurrentState())
		{
			Velocity += Velocity.GetSafeNormal2D() * InitialSlideImpulse;

			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, NULL);

			SetMovementMode(MOVE_Custom, (uint8)EBECustomMovementMode::Slide);

			return true;
		}
	}

	return false;
}

void UBECharacterMovementComponent::OnStartSlide()
{
	GetBECharacterOwner()->K2_OnStartSlide();
}

void UBECharacterMovementComponent::OnEndSlide()
{
	GetBECharacterOwner()->K2_OnEndSlide();
}


bool UBECharacterMovementComponent::IsClimbing() const
{
	return (GetCustomMovementMode() == EBECustomMovementMode::Climb);
}

void UBECharacterMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	bJustTeleported = false;
	float RemainingTime = deltaTime;

	while ((RemainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
	{
		Iterations++;
		bJustTeleported = false;
		const float TimeTick = GetSimulationTimeStep(RemainingTime, Iterations);
		RemainingTime -= TimeTick;

		if (!CanClimbInCurrentState())
		{
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(RemainingTime, Iterations);
			return;
		}


		/////////////////////////////////////////
		// STEP1: 壁を検証

		const float CapsuleRadius			= CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector Start					= UpdatedComponent->GetComponentLocation();
		const FVector End					= Start + UpdatedComponent->GetForwardVector() * CapsuleRadius * 2;
		const FName ProfileName				= TEXT("BlockAll");
		const FCollisionQueryParams Params	= GetBECharacterOwner()->GetIgnoreCharacterParams();
		FHitResult WallHit;
		GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, ProfileName, Params);
		if (!WallHit.IsValidBlockingHit())
		{
			Velocity.Z += ClimbEndJumpUpForce;

			SetMovementMode(MOVE_Falling);
			StartNewPhysics(RemainingTime, Iterations);
			return;
		}


		/////////////////////////////////////////
		// STEP2: 現在の値を保存

		const FQuat PawnRotation  = UpdatedComponent->GetComponentQuat();
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();

		RestorePreAdditiveRootMotionVelocity();

		const FVector OldVelocity = Velocity;


		/////////////////////////////////////////
		// STEP3: Velocity を適用

		Velocity.Z = 0.f;
		CalcVelocity(TimeTick, 0.0, false, GetMaxBrakingDeceleration());
		Velocity.Z = OldVelocity.Z;

		Velocity += FVector(0.f, 0.f, GetGravityZ() * ClimbingGravityScale) * TimeTick;
		Velocity += (-WallHit.Normal) * ClimbWallAttractionForce * TimeTick;

		ApplyRootMotionToVelocity(TimeTick);


		/////////////////////////////////////////
		// STEP4: 移動を適用

		const FVector Delta	= TimeTick * Velocity;
		if (!Delta.IsNearlyZero())
		{
			FHitResult Hit(1.f);
			SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

			if (Hit.Time < 1.f)
			{
				HandleImpact(Hit, deltaTime, Delta);
				SlideAlongSurface(Delta, (1.f - Hit.Time), Hit.Normal, Hit, true);
			}

			if (IsFalling())
			{
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					RemainingTime += TimeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(RemainingTime, Iterations);
				return;
			}
			else if (IsSwimming())
			{
				StartSwimming(OldLocation, OldVelocity, TimeTick, RemainingTime, Iterations);
				return;
			}
		}
		else
		{
			RemainingTime = 0.f;
		}

		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			RemainingTime = 0.f;
			break;
		}

		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / TimeTick;
	}
}

bool UBECharacterMovementComponent::CanClimbInCurrentState() const
{
	return (CanClimb && IsMovingInAir() && !bWantsToCrouch && !bWantsToAim && CharacterOwner->bPressedJump && (Velocity.Z > MinClimbFallSpeed));
}

bool UBECharacterMovementComponent::TryClimb()
{
	if ((GetCustomMovementMode() != EBECustomMovementMode::Climb) && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy))
	{
		if (CanClimbInCurrentState() && !bClimbCooldown)
		{
			const float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
			const FVector Start = UpdatedComponent->GetComponentLocation();
			const FVector End = Start + UpdatedComponent->GetForwardVector() * CapsuleRadius * 2;
			const FName ProfileName = TEXT("BlockAll");
			const FCollisionQueryParams Params = GetBECharacterOwner()->GetIgnoreCharacterParams();

			FHitResult WallHit;
			if (GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, ProfileName, Params))
			{
				if (WallHit.IsValidBlockingHit())
				{
					bClimbCooldown = true;

					Velocity.Z = ClimbInitialSpeed;

					SetMovementMode(MOVE_Custom, (uint8)EBECustomMovementMode::Climb);

					return true;
				}
			}
		}
	}

	return false;
}

void UBECharacterMovementComponent::OnStartClimb()
{
	GetBECharacterOwner()->K2_OnStartClimb();
}

void UBECharacterMovementComponent::OnEndClimb()
{
	GetBECharacterOwner()->K2_OnEndClimb();
}


bool UBECharacterMovementComponent::IsWallRunning() const
{
	return (IsWallRunningRight() || IsWallRunningLeft());
}

bool UBECharacterMovementComponent::IsWallRunningRight() const
{
	return (GetCustomMovementMode() == EBECustomMovementMode::WallRunR);
}

bool UBECharacterMovementComponent::IsWallRunningLeft() const
{
	return (GetCustomMovementMode() == EBECustomMovementMode::WallRunL);
}

bool UBECharacterMovementComponent::TryWallRun()
{
	if (!IsWallRunning() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy))
	{
		if (CanWallRunInCurrentState() && !bWallRunCooldown && GetSpeed2D() > MinEnterWallRunSpeed)
		{
			const float CapsuleRadius	= CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
			const FVector Start			= UpdatedComponent->GetComponentLocation();
			const FName ProfileName		= TEXT("BlockAll");
			const FCollisionQueryParams Params = GetBECharacterOwner()->GetIgnoreCharacterParams();

			FHitResult WallHit;

			const FVector LeftEnd = Start - UpdatedComponent->GetRightVector() * CapsuleRadius * 2;
			if (GetWorld()->LineTraceSingleByProfile(WallHit, Start, LeftEnd, ProfileName, Params))
			{
				if (WallHit.IsValidBlockingHit())
				{
					bWallRunCooldown = true;

					SetMovementMode(MOVE_Custom, (uint8)EBECustomMovementMode::WallRunL);

					return true;
				}
			}

			const FVector RightEnd = Start + UpdatedComponent->GetRightVector() * CapsuleRadius * 2;
			if (GetWorld()->LineTraceSingleByProfile(WallHit, Start, RightEnd, ProfileName, Params))
			{
				if (WallHit.IsValidBlockingHit())
				{
					bWallRunCooldown = true;

					SetMovementMode(MOVE_Custom, (uint8)EBECustomMovementMode::WallRunR);

					return true;
				}
			}
		}
	}

	return false;
}

void UBECharacterMovementComponent::PhysWallRun(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	bJustTeleported = false;
	float RemainingTime = deltaTime;

	while ((RemainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
	{
		Iterations++;
		bJustTeleported = false;
		const float TimeTick = GetSimulationTimeStep(RemainingTime, Iterations);
		RemainingTime -= TimeTick;

		if (!CanWallRunInCurrentState() || (GetSpeed2D() < MinContinueWallRunSpeed))
		{
			if (!CharacterOwner->bPressedJump)
			{
				Velocity.Z	+= WallRunEndJumpUpForce;
				if (IsWallRunningLeft())
				{
					Velocity += (UpdatedComponent->GetRightVector() * WallRunEndJumpOffForce);
				}
				else
				{
					Velocity += (UpdatedComponent->GetRightVector() * (-WallRunEndJumpOffForce));
				}
			}

			SetMovementMode(MOVE_Falling);
			StartNewPhysics(RemainingTime, Iterations);
			return;
		}


		/////////////////////////////////////////
		// STEP1: 壁を検証

		const float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector Start = UpdatedComponent->GetComponentLocation();
		const FName ProfileName = TEXT("BlockAll");
		const FCollisionQueryParams Params = GetBECharacterOwner()->GetIgnoreCharacterParams();

		FHitResult WallHit;

		if (IsWallRunningLeft())
		{
			const FVector LeftEnd = Start - UpdatedComponent->GetRightVector() * CapsuleRadius * 2;
			GetWorld()->LineTraceSingleByProfile(WallHit, Start, LeftEnd, ProfileName, Params);
			if (!WallHit.IsValidBlockingHit())
			{
				SetMovementMode(MOVE_Falling);
				StartNewPhysics(RemainingTime, Iterations);
				return;
			}
		}
		else
		{
			const FVector RightEnd = Start + UpdatedComponent->GetRightVector() * CapsuleRadius * 2;
			GetWorld()->LineTraceSingleByProfile(WallHit, Start, RightEnd, ProfileName, Params);
			if (!WallHit.IsValidBlockingHit())
			{
				SetMovementMode(MOVE_Falling);
				StartNewPhysics(RemainingTime, Iterations);
				return;
			}
		}
		

		/////////////////////////////////////////
		// STEP2: 現在の値を保存

		const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();

		RestorePreAdditiveRootMotionVelocity();

		const FVector OldVelocity = Velocity;


		/////////////////////////////////////////
		// STEP3: Velocity を適用

		Velocity.Z = 0.f;
		ApplyVelocityBraking(TimeTick, 0.0, GetMaxBrakingDeceleration());
		Velocity.Z = OldVelocity.Z;

		Velocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);

		Velocity += FVector(0.f, 0.f, GetGravityZ() * WallRunGravityScale) * TimeTick;
		Velocity += (-WallHit.Normal) * WallRunWallAttractionForce * TimeTick;

		ApplyRootMotionToVelocity(TimeTick);


		/////////////////////////////////////////
		// STEP4: 移動を適用

		const FVector Delta = TimeTick * Velocity;
		if (!Delta.IsNearlyZero())
		{
			FHitResult Hit(1.f);
			SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

			if (Hit.Time < 1.f)
			{
				HandleImpact(Hit, deltaTime, Delta);
				SlideAlongSurface(Delta, (1.f - Hit.Time), Hit.Normal, Hit, true);
			}

			if (IsFalling())
			{
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					RemainingTime += TimeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(RemainingTime, Iterations);
				return;
			}
			else if (IsSwimming())
			{
				StartSwimming(OldLocation, OldVelocity, TimeTick, RemainingTime, Iterations);
				return;
			}
		}
		else
		{
			RemainingTime = 0.f;
		}

		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			RemainingTime = 0.f;
			break;
		}

		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / TimeTick;
	}
}

bool UBECharacterMovementComponent::CanWallRunInCurrentState() const
{
	return (CanWallRun && IsMovingInAir() && !bWantsToCrouch && !bWantsToAim &&CharacterOwner->bPressedJump && (Velocity.Z > MinWallRunFallSpeed));
}

void UBECharacterMovementComponent::OnStartWallRun()
{
	GetBECharacterOwner()->K2_OnStartWallRun();
}

void UBECharacterMovementComponent::OnEndWallRun()
{
	GetBECharacterOwner()->K2_OnEndWallRun();
}

#pragma endregion

#pragma region CMC Movement Attribute

void UBECharacterMovementComponent::HandleGravityScaleChanged(const FOnAttributeChangeData& ChangeData)
{
	GravityScale = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleGroundFrictionChanged(const FOnAttributeChangeData& ChangeData)
{
	GroundFriction = ChangeData.NewValue;
}


void UBECharacterMovementComponent::HandleOverallSpeedMultiplierChanged(const FOnAttributeChangeData& ChangeData)
{
	OverallMaxSpeedMultiplier = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleWalkSpeedChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxWalkSpeed = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleWalkSpeedCrouchedChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxWalkSpeedCrouched = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleWalkSpeedRunningChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxWalkSpeedRunning = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleWalkSpeedAimingChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxWalkSpeedAiming = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleSwimSpeedChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxSwimSpeed = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleFlySpeedChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxFlySpeed = ChangeData.NewValue;
}


void UBECharacterMovementComponent::HandleJumpPowerChanged(const FOnAttributeChangeData& ChangeData)
{
	JumpZVelocity = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleAirControlChanged(const FOnAttributeChangeData& ChangeData)
{
	AirControl = ChangeData.NewValue;
}

#pragma endregion
