// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BECharacterMovementComponent.h"

#include "Character/Component/BEPawnBasicComponent.h"
#include "Character/Movement/BECharacterMovementFragment.h"
#include "Character/BECharacter.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "Ability/Attribute/BEMovementSet.h"
#include "BELogChannels.h"
#include "GameplayTag/BETags_Status.h"
#include "GameplayTag/BETags_Flag.h"
#include "GameplayTag/BETags_InitState.h"

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
#include "Stats/Stats2.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
#include "Curves/CurveVector.h"
#include "Components/GameFrameworkComponentManager.h"

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

	Saved_bWantsToSprint		= false;
	Saved_bWantsToTarget	= false;
}

uint8 UBECharacterMovementComponent::FSavedMove_BECharacter::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToSprint)
	{
		Result |= FLAG_Sprint;
	}

	if (Saved_bWantsToTarget)
	{
		Result |= FLAG_Target;
	}

	return Result;
}

bool UBECharacterMovementComponent::FSavedMove_BECharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	const FSavedMove_BECharacter* NewSavedMove = static_cast<FSavedMove_BECharacter*>(NewMove.Get());

	if (Saved_bWantsToSprint != NewSavedMove->Saved_bWantsToSprint)
	{
		return false;
	}

	if (Saved_bWantsToTarget != NewSavedMove->Saved_bWantsToTarget)
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
		Saved_bWantsToSprint		= CharacterMovement->bWantsToSprint;
		Saved_bWantsToTarget		= CharacterMovement->bWantsToTarget;
	}
}

void UBECharacterMovementComponent::FSavedMove_BECharacter::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UBECharacterMovementComponent* CharacterMovement = Cast<UBECharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->bWantsToSprint		= Saved_bWantsToSprint;
		CharacterMovement->bWantsToTarget	= Saved_bWantsToTarget;
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

#pragma region CharacterMovementComponent

const FName UBECharacterMovementComponent::NAME_ActorFeatureName("CharacterMovement");

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


void UBECharacterMovementComponent::OnRegister()
{
	Super::OnRegister();

	// コンポーネントが GameWorld に存在する際に　InitStateSystem に登録する
	RegisterInitStateFeature();
}

void UBECharacterMovementComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

void UBECharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// すべての Feature への変更をリッスンする
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// InisStateSystem にこのコンポーネントがスポーンしたことを知らせる。
	ensure(TryToChangeInitState(TAG_InitState_Spawned));

	// 残りの初期化を行う
	CheckDefaultInitialization();
}

void UBECharacterMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeFromAbilitySystem();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
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

	MovementSet = Cast<UBEMovementSet>(AbilitySystemComponent->InitStats(UBEMovementSet::StaticClass(), nullptr));
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
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetWalkSpeedSprintingAttribute()).AddUObject(this, &ThisClass::HandleWalkSpeedSprintingChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetWalkSpeedTargetingAttribute()).AddUObject(this, &ThisClass::HandleWalkSpeedTargetingChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetSwimSpeedAttribute()).AddUObject(this, &ThisClass::HandleSwimSpeedChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetFlySpeedAttribute()).AddUObject(this, &ThisClass::HandleFlySpeedChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetJumpPowerAttribute()).AddUObject(this, &ThisClass::HandleJumpPowerChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetAirControlAttribute()).AddUObject(this, &ThisClass::HandleAirControlChanged);

	if (CharacterOwner->HasAuthority())
	{
		// アトリビュートの初期値を設定
		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetGravityScaleAttribute(), GravityScale);
		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetGroundFrictionAttribute(), GroundFriction);

		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetOverallSpeedMultiplierAttribute(), OverallMaxSpeedMultiplier);
		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetWalkSpeedAttribute(), MaxWalkSpeed);
		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetWalkSpeedCrouchedAttribute(), MaxWalkSpeedCrouched);
		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetWalkSpeedSprintingAttribute(), MaxWalkSpeedSprinting);
		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetWalkSpeedTargetingAttribute(), MaxWalkSpeedTargeting);
		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetSwimSpeedAttribute(), MaxSwimSpeed);
		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetFlySpeedAttribute(), MaxFlySpeed);

		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetJumpPowerAttribute(), JumpZVelocity);
		AbilitySystemComponent->SetNumericAttributeBase(UBEMovementSet::GetAirControlAttribute(), AirControl);

		InitializeGameplayTags();
	}
}

void UBECharacterMovementComponent::UninitializeFromAbilitySystem()
{
	MovementSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void UBECharacterMovementComponent::InitializeGameplayTags()
{
	check(AbilitySystemComponent);

	// AbilitySystem から MovementMode に関する Tag を削除する
	AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_Walking, 0);
	AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_NavWalking, 0);
	AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_Falling, 0);
	AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_Swimming, 0);
	AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_Flying, 0);

	for (const UBECharacterMovementFragment* Fragment : Fragments)
	{
		if (Fragment != nullptr)
		{
			FGameplayTag ModeTag = Fragment->GetMovementModeTag();
			if (ModeTag.IsValid())
			{
				AbilitySystemComponent->SetLooseGameplayTagCount(ModeTag, 0);
			}
		}
	}

	SetMovementModeTag(MovementMode, CustomMovementMode, true);
}


bool UBECharacterMovementComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	/**
	 * [InitState None] -> [InitState Spawned]
	 *
	 *  Character が有効
	 */
	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		if (CharacterOwner != nullptr)
		{
			return true;
		}
	}

	/**
	 * [InitState Spawned] -> [InitState DataAvailable]
	 *
	 *  Controller が有効
	 */
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		// シミュレーションでなく、かつ Controller が PlayerState を所有しているか
		if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController();
			if (Controller == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 *
	 *  他のすべての Feature が DataAvailable に到達している
	 */
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		// CharacterBasicComponent が DataInitialized に到達しているか
		// つまり、他のすべての Feature が DataAvailable に到達しているか
		return Manager->HasFeatureReachedInitState(CharacterOwner, UBEPawnBasicComponent::NAME_ActorFeatureName, TAG_InitState_DataInitialized);
	}

	/**
	 * [InitState DataInitialized] -> [InitState GameplayReady]
	 *
	 *  無条件で許可
	 */
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UBECharacterMovementComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 *
	 *  AbilitySystem に Movement を初期化
	 */
	if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		if (!ensure(CharacterOwner))
		{
			return;
		}

		if (UBEPawnBasicComponent* CharacterBasic = UBEPawnBasicComponent::FindPawnBasicComponent(CharacterOwner))
		{
			InitializeWithAbilitySystem(CharacterBasic->GetBEAbilitySystemComponent());
		}
	}
}

void UBECharacterMovementComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// CharacterBasicComponent が DataInitialized に到達しているか
	// つまり、他のすべての Feature が DataAvailable に到達しているか
	if (Params.FeatureName == UBEPawnBasicComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataInitialized)
		{
			CheckDefaultInitialization();
		}
	}
}

void UBECharacterMovementComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}


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

void UBECharacterMovementComponent::HandleWalkSpeedSprintingChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxWalkSpeedSprinting = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleWalkSpeedTargetingChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxWalkSpeedTargeting = ChangeData.NewValue;
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


void UBECharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAcceleration;
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

void UBECharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToSprint = (Flags & FSavedMove_BECharacter::FLAG_Sprint) != 0;

	bWantsToTarget = (Flags & FSavedMove_BECharacter::FLAG_Target) != 0;
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

		const bool bIsSprinting = IsSprinting();
		if (bIsSprinting && (!bWantsToSprint || !CanSprintInCurrentState()))
		{
			UnSprint(false);
		}
		else if (!bIsSprinting && bWantsToSprint && CanSprintInCurrentState())
		{
			Sprint(false);
		}

		const bool bIsTargeting = IsTargeting();
		if (bIsTargeting && (!bWantsToTarget || !CanTargetInCurrentState()))
		{
			UnTarget(false);
		}
		else if (!bIsTargeting && bWantsToTarget && CanTargetInCurrentState())
		{
			Target(false);
		}

		for (UBECharacterMovementFragment* Fragment : Fragments)
		{
			if (Fragment != nullptr)
			{
				Fragment->TryStartMovement(this);
			}
		}
	}
}


bool UBECharacterMovementComponent::CanCrouchInCurrentState() const
{
	if (!CanEverCrouch())
	{
		return false;
	}

	return ((IsFalling() && bCanCrouchInAir) || IsMovingOnGround()) && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

bool UBECharacterMovementComponent::CanAttemptJump() const
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Flag_JumpBlocked))
		{
			return false;
		}
	}

	return IsJumpAllowed() && IsMovingOnGround();
}

bool UBECharacterMovementComponent::IsMovingOnGround() const
{
	if (!Super::IsMovingOnGround())
	{
		return false;
	}

	if (MovementMode == MOVE_Custom)
	{
		if (UBECharacterMovementFragment* Fragment = Fragments[CustomMovementMode])
		{
			return (Fragment->GetMovementSpace() == EBECustomMovementSpace::OnGround);
		}
	}

	return true;
}

bool UBECharacterMovementComponent::IsMovingInAir() const
{
	if (!(IsFalling() || IsFlying()))
	{
		return false;
	}

	if (MovementMode == MOVE_Custom)
	{
		if (UBECharacterMovementFragment* Fragment = Fragments[CustomMovementMode])
		{
			return (Fragment->GetMovementSpace() == EBECustomMovementSpace::InAir);
		}
	}

	return true;
}


FRotator UBECharacterMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Flag_MovementBlocked))
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
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Flag_MovementBlocked))
		{
			return 0.0f;
		}
	}

	EMovementMode LocMoveMode = MovementMode;
	float SuggestMaxSpeed = 0.0;

	if ((LocMoveMode == MOVE_Walking) || (LocMoveMode == MOVE_NavWalking))
	{
		if (IsTargeting())
		{
			if (IsCrouching())
			{
				return FMath::Min(MaxWalkSpeedCrouched, MaxWalkSpeedTargeting);
			}
			else
			{
				return MaxWalkSpeedTargeting;
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
				if (IsSprinting())
				{
					SuggestMaxSpeed = MaxWalkSpeedSprinting;
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
		if (IsSprinting())
		{
			SuggestMaxSpeed = MaxWalkSpeedSprinting;
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
		if (const UBECharacterMovementFragment* Fragment = Fragments[CustomMovementMode])
		{
			Fragment->GetMaxMoveSpeed(this);
		}
	}

	return SuggestMaxSpeed * OverallMaxSpeedMultiplier;
}

float UBECharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode == MOVE_Custom)
	{
		if (const UBECharacterMovementFragment* Fragment = Fragments[CustomMovementMode])
		{
			Fragment->GetMaxBrakingDeceleration(this);
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


void UBECharacterMovementComponent::SetMovementModeTag(EMovementMode InMovementMode, uint8 InCustomMovementMode, bool bTagEnabled)
{
	if (AbilitySystemComponent)
	{
		const int32 NewCount = (bTagEnabled ? 1 : 0);

		if (InMovementMode == MOVE_Walking)
		{
			AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_Walking, NewCount);
		}
		else if (InMovementMode == MOVE_NavWalking)
		{
			AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_NavWalking, NewCount);
		}
		else if (InMovementMode == MOVE_Falling)
		{
			AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_Falling, NewCount);
		}
		else if (InMovementMode == MOVE_Swimming)
		{
			AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_Swimming, NewCount);
		}
		else if (InMovementMode == MOVE_Flying)
		{
			AbilitySystemComponent->SetLooseGameplayTagCount(TAG_Movement_Mode_Flying, NewCount);
		}
		else if (InMovementMode == MOVE_Custom)
		{
			if (const UBECharacterMovementFragment* Fragment = Fragments[CustomMovementMode])
			{
				const FGameplayTag MovementModeTag = Fragment->GetMovementModeTag();
				if (MovementModeTag.IsValid())
				{
					AbilitySystemComponent->SetLooseGameplayTagCount(MovementModeTag, NewCount);
				}
			}
		}
	}
}

void UBECharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	SetMovementModeTag(PreviousMovementMode, PreviousCustomMode, false);
	SetMovementModeTag(MovementMode, CustomMovementMode, true);

	bCrouchMaintainsBaseLocation = true;

	if (MovementMode == MOVE_Custom)
	{
		if (UBECharacterMovementFragment* Fragment = Fragments[CustomMovementMode])
		{
			Fragment->OnMovementStart(this);
		}
	}
	else if (PreviousMovementMode == MOVE_Custom)
	{
		if (UBECharacterMovementFragment* Fragment = Fragments[CustomMovementMode])
		{
			Fragment->OnMovementEnd(this);
		}
	}
	else if (MovementMode == MOVE_Walking)
	{
		if (PreviousMovementMode == MOVE_Falling)
		{
			CharacterOwner->StopJumping();

			for (UBECharacterMovementFragment* Fragment : Fragments)
			{
				if (Fragment != nullptr)
				{
					Fragment->OnLanded(this);
				}
			}
		}
	}
}

void UBECharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	if (UBECharacterMovementFragment* Fragment = Fragments[CustomMovementMode])
	{
		Fragment->PhysMovement(this, deltaTime, Iterations);
	}
}


bool UBECharacterMovementComponent::IsSprinting() const
{
	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		return BEChara->bIsSprinting;
	}

	return false;
}

bool UBECharacterMovementComponent::CanSprintInCurrentState() const
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Flag_SprintBlocked))
		{
			return false;
		}
	}

	return !IsTargeting() && (IsFalling() || IsMovingOnGround());
}

void UBECharacterMovementComponent::Sprint(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		if (!bClientSimulation)
		{
			BEChara->bIsSprinting = true;
		}

		BEChara->OnStartSprint();
	}
}

void UBECharacterMovementComponent::UnSprint(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		if (!bClientSimulation)
		{
			BEChara->bIsSprinting = false;
		}

		BEChara->OnEndSprint();
	}
}


bool UBECharacterMovementComponent::IsTargeting() const
{
	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		return BEChara->bIsTargeting;
	}

	return false;
}

bool UBECharacterMovementComponent::CanTargetInCurrentState() const
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Flag_TargetBlocked))
		{
			return false;
		}
	}

	return (IsMovingInAir() || IsMovingOnGround());
}

void UBECharacterMovementComponent::Target(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		if (!bClientSimulation)
		{
			BEChara->bIsTargeting = true;
		}

		BEChara->OnStartTarget();
	}
}

void UBECharacterMovementComponent::UnTarget(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (ABECharacter* BEChara = GetBECharacterOwner())
	{
		if (!bClientSimulation)
		{
			BEChara->bIsTargeting = false;
		}

		BEChara->OnEndTarget();
	}
}


UBECharacterMovementComponent* UBECharacterMovementComponent::FindCharacterMovementComponent(const ABECharacter* Character)
{
	return (Character ? Character->FindComponentByClass<UBECharacterMovementComponent>() : nullptr);
}

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

#pragma endregion
