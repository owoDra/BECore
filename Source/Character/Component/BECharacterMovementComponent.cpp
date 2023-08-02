// Copyright owoDra

#include "BECharacterMovementComponent.h"

#include "Character/Animation/BECharacterAnimCurveNameStatics.h"
#include "Character/Animation/BECharacterAnimInstance.h"
#include "Character/Component/BEPawnBasicComponent.h"
#include "Character/Movement/BECharacterCustomMovement.h"
#include "Character/Movement/BECharacterMovementData.h"
#include "Character/Movement/BEMovementMathLibrary.h"
#include "Character/BEPawnMeshAssistInterface.h"
#include "Character/BECharacter.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "Ability/Attribute/BEMovementSet.h"
#include "GameplayTag/BETags_Status.h"
#include "GameplayTag/BETags_Flag.h"
#include "GameplayTag/BETags_InitState.h"
#include "BELogChannels.h"
#include "Development/BEDeveloperStatGroup.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameNetworkManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/GameFrameworkComponentManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterMovementComponent)


/**
 * FBECharacterNetworkMoveData
 */
#pragma region FBECharacterNetworkMoveData

FBECharacterNetworkMoveData::FBECharacterNetworkMoveData()
{
	RotationMode	= TAG_Status_RotationMode_ViewDirection;
	Stance			= TAG_Status_Stance_Standing;
	Gait			= TAG_Status_Gait_Walking;
}

void FBECharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(Move, MoveType);

	const auto& SavedMove{ static_cast<const FBESavedMove&>(Move) };

	RotationMode	= SavedMove.RotationMode;
	Stance			= SavedMove.Stance;
	Gait			= SavedMove.Gait;
}

bool FBECharacterNetworkMoveData::Serialize(UCharacterMovementComponent& Movement, FArchive& Archive, UPackageMap* Map, const ENetworkMoveType MoveType) 
{
	Super::Serialize(Movement, Archive, Map, MoveType);

	NetSerializeOptionalValue(Archive.IsSaving(), Archive, RotationMode	, TAG_Status_RotationMode_ViewDirection.GetTag(), Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, Stance		, TAG_Status_Stance_Standing.GetTag()			, Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, Gait			, TAG_Status_Gait_Walking.GetTag()				, Map);

	return !Archive.IsError();
}

#pragma endregion


/**
 * FBECharacterNetworkMoveDataContainer
 */
#pragma region FBECharacterNetworkMoveDataContainer

FBECharacterNetworkMoveDataContainer::FBECharacterNetworkMoveDataContainer()
{
	NewMoveData		= &MoveData[0];
	PendingMoveData = &MoveData[1];
	OldMoveData		= &MoveData[2];
}

#pragma endregion


/**
 * FBESavedMove
 */

#pragma region FBESavedMove

FBESavedMove::FBESavedMove()
{
	RotationMode	= TAG_Status_RotationMode_ViewDirection;
	Stance			= TAG_Status_Stance_Standing;
	Gait			= TAG_Status_Gait_Walking;
}

void FBESavedMove::Clear()
{
	Super::Clear();

	RotationMode	= TAG_Status_RotationMode_ViewDirection;
	Stance			= TAG_Status_Stance_Standing;
	Gait			= TAG_Status_Gait_Walking;
}

void FBESavedMove::SetMoveFor(ACharacter* Character, float NewDeltaTime, const FVector& NewAcceleration, FNetworkPredictionData_Client_Character& PredictionData)
{
	Super::SetMoveFor(Character, NewDeltaTime, NewAcceleration, PredictionData);

	const auto* Movement{ Cast<UBECharacterMovementComponent>(Character->GetCharacterMovement()) };
	if (IsValid(Movement))
	{
		RotationMode	= Movement->RotationMode;
		Stance			= Movement->Stance;
		Gait			= Movement->Gait;
	}
}

bool FBESavedMove::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const
{
	const auto* NewMove{ static_cast<FBESavedMove*>(NewMovePtr.Get()) };

	return RotationMode == NewMove->RotationMode && Stance == NewMove->Stance && Gait == NewMove->Gait && Super::CanCombineWith(NewMovePtr, Character, MaxDelta);
}

void FBESavedMove::CombineWith(const FSavedMove_Character* PreviousMove, ACharacter* Character, APlayerController* Player, const FVector& PreviousStartLocation)
{
	// Super::CombineWith() を呼び出すと、キャラクタの回転が前の移動の回転に強制的に変更されます。
	// ABECharacter クラスで行った回転の変更が消去されるため、望ましくありません。そのため、回転を変更しないようにします、
	// Super::CombineWith() を呼び出した後、それらを復元します。

	const auto OriginalRotation{ PreviousMove->StartRotation };
	const auto OriginalRelativeRotation{ PreviousMove->StartAttachRelativeRotation };

	const auto* UpdatedComponent{ Character->GetCharacterMovement()->UpdatedComponent.Get() };

	const_cast<FSavedMove_Character*>(PreviousMove)->StartRotation				 = UpdatedComponent->GetComponentRotation();
	const_cast<FSavedMove_Character*>(PreviousMove)->StartAttachRelativeRotation = UpdatedComponent->GetRelativeRotation();

	Super::CombineWith(PreviousMove, Character, Player, PreviousStartLocation);

	const_cast<FSavedMove_Character*>(PreviousMove)->StartRotation				 = OriginalRotation;
	const_cast<FSavedMove_Character*>(PreviousMove)->StartAttachRelativeRotation = OriginalRelativeRotation;
}

void FBESavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	auto* Movement{ Cast<UBECharacterMovementComponent>(Character->GetCharacterMovement()) };
	if (IsValid(Movement))
	{
		Movement->RotationMode	= RotationMode;
		Movement->Stance		= Stance;
		Movement->Gait			= Gait;

		Movement->RefreshGaitConfigs();
	}
}

#pragma endregion


/**
 * FBENetworkPredictionData
 */

#pragma region FBENetworkPredictionData

FBENetworkPredictionData::FBENetworkPredictionData(const UCharacterMovementComponent& Movement)
	: Super(Movement)
{
}

FSavedMovePtr FBENetworkPredictionData::AllocateNewMove()
{
	return MakeShared<FBESavedMove>();
}

#pragma endregion


//////////////////////////////////////////////
//	キャラクタームーブメント

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

	// ステート設定
	LocomotionMode			= TAG_Status_LocomotionMode_OnGround;
	DesiredRotationMode		= TAG_Status_RotationMode_ViewDirection;
	RotationMode			= TAG_Status_RotationMode_ViewDirection;
	DesiredStance			= TAG_Status_Stance_Standing;
	Stance					= TAG_Status_Stance_Standing;
	DesiredGait				= TAG_Status_Gait_Walking;
	Gait					= TAG_Status_Gait_Walking;
}

#if WITH_EDITOR

bool UBECharacterMovementComponent::CanEditChange(const FProperty* Property) const
{
	return Super::CanEditChange(Property) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, RotationRate) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerDesiredRotation) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bOrientRotationToMovement);
}

#endif


#pragma region Initialize and Deinitialize

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
	ensureMsgf(!bUseControllerDesiredRotation && !bOrientRotationToMovement,
		TEXT("bUseControllerDesiredRotation and bOrientRotationToMovement must be turned false!"));

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
	auto* Owner{ GetOwner() };
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
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetGravityScaleAttribute()).AddUObject(this, &ThisClass::HandleGravityScaleScaleChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetGroundFrictionScaleAttribute()).AddUObject(this, &ThisClass::HandleGroundFrictionScaleChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetMoveSpeedScaleAttribute()).AddUObject(this, &ThisClass::HandleMoveSpeedScaleChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetJumpPowerScaleAttribute()).AddUObject(this, &ThisClass::HandleJumpPowerScaleChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetAirControlScaleAttribute()).AddUObject(this, &ThisClass::HandleAirControlScaleChanged);
}

void UBECharacterMovementComponent::UninitializeFromAbilitySystem()
{
	MovementSet = nullptr;
	AbilitySystemComponent = nullptr;
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
		if (!CharacterOwner)
		{
			return false;
		}

		if (!MovementData)
		{
			return false;
		}

		return true;
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
			auto* Controller{ GetController() };
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

		if (!ensure(MovementData))
		{
			return;
		}

		UpdateLocomotionConfigs();

		SetReplicatedViewRotation(GetBECharacterOwner()->GetViewRotationSuperClass());

		ViewState.NetworkSmoothing.InitialRotation = ReplicatedViewRotation;
		ViewState.NetworkSmoothing.Rotation = ReplicatedViewRotation;
		ViewState.Rotation = ReplicatedViewRotation;
		ViewState.PreviousYawAngle = UE_REAL_TO_FLOAT(ReplicatedViewRotation.Yaw);

		const auto& ActorTransform{ CharacterOwner->GetActorTransform() };

		LocomotionState.Location = ActorTransform.GetLocation();
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = ActorTransform.GetRotation().Rotator();
		LocomotionState.PreviousYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);

		UpdateTargetYawAngleUsingLocomotionRotation();

		LocomotionState.InputYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);
		LocomotionState.VelocityYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);

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
	static const TArray<FGameplayTag> StateChain{
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}

#pragma endregion


#pragma region Attributes

void UBECharacterMovementComponent::HandleGravityScaleScaleChanged(const FOnAttributeChangeData& ChangeData)
{
	GravityScaleScale = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleGroundFrictionScaleChanged(const FOnAttributeChangeData& ChangeData)
{
	GroundFrictionScale = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleMoveSpeedScaleChanged(const FOnAttributeChangeData& ChangeData)
{
	MoveSpeedScale = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleJumpPowerScaleChanged(const FOnAttributeChangeData& ChangeData)
{
	JumpPowerScale = ChangeData.NewValue;
}

void UBECharacterMovementComponent::HandleAirControlScaleChanged(const FOnAttributeChangeData& ChangeData)
{
	AirControlScale = ChangeData.NewValue;
}

#pragma endregion


#pragma region Replication

void UBECharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	Parameters.Condition = COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredStance, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredGait, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredRotationMode, Parameters);

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InputDirection, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredVelocityYawAngle, Parameters);
}

FNetworkPredictionData_Client* UBECharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		auto* MutableThis{ const_cast<UBECharacterMovementComponent*>(this) };

		MutableThis->ClientPredictionData = new FBENetworkPredictionData(*this);
	}

	return ClientPredictionData;
}

#pragma endregion


#pragma region Movement Base

void UBECharacterMovementComponent::UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation)
{
	const auto& BasedMovement{ CharacterOwner->GetBasedMovement() };

	FVector MovementBaseLocation;
	FQuat MovementBaseRotation;

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName,
		MovementBaseLocation, MovementBaseRotation);

	if (!OldBaseQuat.Equals(MovementBaseRotation, UE_SMALL_NUMBER))
	{
		const FRotator DeltaRotation{ (MovementBaseRotation * OldBaseQuat.Inverse()).Rotator() };
		FRotator NewControlRotation{ CharacterOwner->Controller->GetControlRotation() };

		NewControlRotation.Pitch += DeltaRotation.Pitch;
		NewControlRotation.Yaw += DeltaRotation.Yaw;
		NewControlRotation.Normalize();

		CharacterOwner->Controller->SetControlRotation(NewControlRotation);
	}
}

void UBECharacterMovementComponent::UpdateUsingAbsoluteRotation() const
{
	const auto bNotDedicatedServer{ !IsNetMode(NM_DedicatedServer) };

	const auto bAutonomousProxyOnListenServer{ IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy };

	const auto bNonLocallyControllerCharacterWithURO{ CharacterOwner->GetMesh()->ShouldUseUpdateRateOptimizations() && !IsValid(CharacterOwner->GetInstigatorController<APlayerController>()) };

	CharacterOwner->GetMesh()->SetUsingAbsoluteRotation(bNotDedicatedServer && (bAutonomousProxyOnListenServer || bNonLocallyControllerCharacterWithURO));
}

void UBECharacterMovementComponent::UpdateVisibilityBasedAnimTickOption() const
{
	const auto DefaultTickOption{ CharacterOwner->GetClass()->GetDefaultObject<ACharacter>()->GetMesh()->VisibilityBasedAnimTickOption };

	const auto TargetTickOption{ 
		(IsNetMode(NM_Standalone) || CharacterOwner->GetLocalRole() <= ROLE_AutonomousProxy || CharacterOwner->GetRemoteRole() != ROLE_AutonomousProxy)
		? EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered : EVisibilityBasedAnimTickOption::AlwaysTickPose
	};

	CharacterOwner->GetMesh()->VisibilityBasedAnimTickOption = TargetTickOption <= DefaultTickOption ? TargetTickOption : DefaultTickOption;
}

void UBECharacterMovementComponent::UpdateMovementBase()
{
	const auto& BasedMovement{ CharacterOwner->GetBasedMovement() };

	if (BasedMovement.MovementBase != MovementBase.Primitive || BasedMovement.BoneName != MovementBase.BoneName)
	{
		MovementBase.Primitive = BasedMovement.MovementBase;
		MovementBase.BoneName = BasedMovement.BoneName;
		MovementBase.bBaseChanged = true;
	}
	else
	{
		MovementBase.bBaseChanged = false;
	}

	MovementBase.bHasRelativeLocation = BasedMovement.HasRelativeLocation();
	MovementBase.bHasRelativeRotation = MovementBase.bHasRelativeLocation && BasedMovement.bRelativeRotation;

	const auto PreviousRotation{ MovementBase.Rotation };

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName, MovementBase.Location, MovementBase.Rotation);

	MovementBase.DeltaRotation = (MovementBase.bHasRelativeLocation && !MovementBase.bBaseChanged) ? (MovementBase.Rotation * PreviousRotation.Inverse()).Rotator() : FRotator::ZeroRotator;
}

void UBECharacterMovementComponent::UpdateAnimInstanceMovement()
{
	if (!CharacterOwner->GetMesh()->bRecentlyRendered &&
		(CharacterOwner->GetMesh()->VisibilityBasedAnimTickOption > EVisibilityBasedAnimTickOption::AlwaysTickPose))
	{
		UBEAnimInstance* BEAnimIns{ nullptr };

		IBEPawnMeshAssistInterface::Execute_GetMainAnimInstance(CharacterOwner, BEAnimIns);

		if (UBECharacterAnimInstance* BECharaAnimIns = Cast<UBECharacterAnimInstance>(BEAnimIns))
		{
			BECharaAnimIns->MarkPendingUpdate();
		}
	}
}

#pragma endregion


#pragma region Locomotion Mode

void UBECharacterMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	if (!bMovementModeLocked)
	{
		Super::SetMovementMode(NewMovementMode, NewCustomMode);
	}
}

void UBECharacterMovementComponent::SetLocomotionMode(const FGameplayTag& NewLocomotionMode)
{
	if (LocomotionMode != NewLocomotionMode)
	{
		LocomotionMode = NewLocomotionMode;

		RefreshGaitConfigs();
	}
}

void UBECharacterMovementComponent::UpdateLocomotionConfigs()
{
	// 現在の LocomotionMode の Configs を取得

	const auto& LocomotionModeConfigs{ MovementData->GetAllowedLocomotionMode(LocomotionMode) };

	// LocomotionModeConfigs から現在の DesiredRotationMode に基づいて Tag と Configs を取得し RotationMode を更新する

	auto AllowedRotationMode{ DesiredRotationMode };
	const auto& RotationModeConfigs{ LocomotionModeConfigs.GetAllowedRotationMode(this, DesiredRotationMode, AllowedRotationMode) };
	SetRotationMode(AllowedRotationMode);
	
	// RotationModeConfigs から現在の Stance の Configs を取得

	auto AllowedStance{ DesiredStance };
	const auto& StanceConfigs{ RotationModeConfigs.GetAllowedStance(DesiredStance, AllowedStance) };
	SetStance(AllowedStance);

	// StanceConfigs から現在の DesiredGait に基づいて Tag と Configs を取得 Gait を更新する

	auto AllowedGait{ DesiredGait };
	const auto& GaitConfigs{ StanceConfigs.GetAllowedGait(this, DesiredGait, AllowedGait) };
	SetGait(AllowedGait);

	if (bShouldUpdateGaitConfigs)
	{
		RefreshGaitConfigs(GaitConfigs);

		bShouldUpdateGaitConfigs = false;
	}
}

void UBECharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	bCrouchMaintainsBaseLocation = true;

	// 現在の MovementMode が Walking または NavWalking

	if ((MovementMode == MOVE_Walking) || (MovementMode == MOVE_NavWalking))
	{
		SetLocomotionMode(TAG_Status_LocomotionMode_OnGround);
	}

	// 現在の MovementMode が Falling

	else if (MovementMode == MOVE_Falling)
	{
		SetLocomotionMode(TAG_Status_LocomotionMode_InAir);
	}

	// 現在の MovementMode が Swimming

	else if (MovementMode == MOVE_Swimming)
	{
		SetLocomotionMode(TAG_Status_LocomotionMode_InWater);
	}

	// 現在の MovementMode が Custom

	else if (MovementMode == MOVE_Custom)
	{
		const auto CustomMovementIndex{ static_cast<int32>(PreviousCustomMode) };

		if (MovementData->CustomMovements.IsValidIndex(CustomMovementIndex))
		{
			if (UBECharacterCustomMovement* CustomMovement = MovementData->CustomMovements[CustomMovementIndex])
			{
				CustomMovement->OnMovementStart(this);

				SetLocomotionMode(CustomMovement->GetLocomotionMode());
			}
		}
	}

	// 前回の MovementMode が Custom

	if (PreviousMovementMode == MOVE_Custom)
	{
		const auto CustomMovementIndex{ static_cast<int32>(PreviousCustomMode) };

		if (MovementData->CustomMovements.IsValidIndex(CustomMovementIndex))
		{
			if (UBECharacterCustomMovement* CustomMovement = MovementData->CustomMovements[CustomMovementIndex])
			{
				CustomMovement->OnMovementEnd(this);
			}
		}
	}
}

#pragma endregion


#pragma region Desired Rotation Mode

void UBECharacterMovementComponent::SetDesiredRotationMode(FGameplayTag NewDesiredRotationMode)
{
	if (DesiredRotationMode != NewDesiredRotationMode)
	{
		DesiredRotationMode = NewDesiredRotationMode;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredRotationMode, this);

		if (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetDesiredRotationMode(DesiredRotationMode);
		}
	}
}

void UBECharacterMovementComponent::Server_SetDesiredRotationMode_Implementation(FGameplayTag NewDesiredRotationMode)
{
	SetDesiredRotationMode(NewDesiredRotationMode);
}

#pragma endregion


#pragma region Rotation Mode

void UBECharacterMovementComponent::SetRotationMode(FGameplayTag NewRotationMode)
{
	if (RotationMode != NewRotationMode)
	{
		RotationMode = NewRotationMode;

		bShouldUpdateGaitConfigs = true;
	}
}

#pragma endregion


#pragma region Desired Stance

void UBECharacterMovementComponent::SetDesiredStance(FGameplayTag NewDesiredStance)
{
	if (DesiredStance != NewDesiredStance)
	{
		DesiredStance = NewDesiredStance;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredStance, this);

		if (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetDesiredStance(DesiredStance);
		}
	}
}

void UBECharacterMovementComponent::Server_SetDesiredStance_Implementation(FGameplayTag NewDesiredStance)
{
	SetDesiredStance(NewDesiredStance);
}

#pragma endregion 


#pragma region Stance

void UBECharacterMovementComponent::SetStance(FGameplayTag NewStance)
{
	if (NewStance == TAG_Status_Stance_Standing)
	{
		CharacterOwner->UnCrouch();
	}
	else if (NewStance == TAG_Status_Stance_Crouching)
	{
		CharacterOwner->Crouch();
	}

	// キャラクターがまだしゃがみまたは立ち状態になっていない場合はそれに応じて DesiredStance を上書きする

	NewStance = IsCrouching() ? TAG_Status_Stance_Crouching : TAG_Status_Stance_Standing;

	if (Stance != NewStance)
	{
		Stance = NewStance;

		bShouldUpdateGaitConfigs = true;
	}
}

bool UBECharacterMovementComponent::CanCrouchInCurrentState() const
{
	if (!CanEverCrouch())
	{
		return false;
	}

	return UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

#pragma endregion


#pragma region Desired Gait

void UBECharacterMovementComponent::SetDesiredGait(FGameplayTag NewDesiredGait)
{
	if (DesiredGait != NewDesiredGait)
	{
		DesiredGait = NewDesiredGait;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredGait, this);

		if (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetDesiredGait(DesiredGait);
		}
	}
}

void UBECharacterMovementComponent::Server_SetDesiredGait_Implementation(FGameplayTag NewDesiredGait)
{
	SetDesiredGait(NewDesiredGait);
}

#pragma endregion


#pragma region Gait

void UBECharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	FRotator BaseRotationSpeed;
	if (!bIgnoreBaseRotation && TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// 速度をオフセットして、動きのベースに対する相対的な速度を維持する
		Velocity = (BaseRotationSpeed * DeltaTime).RotateVector(Velocity);
	}

	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

float UBECharacterMovementComponent::GetMaxSpeed() const
{
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->GetTagCount(TAG_Flag_MovementBlocked) > 0)
		{
			return 0.0;
		}
	}

	return Super::GetMaxSpeed() * MoveSpeedScale;
}


void UBECharacterMovementComponent::SetGait(FGameplayTag NewGait)
{
	if (Gait != NewGait)
	{
		Gait = NewGait;

		bShouldUpdateGaitConfigs = true;
	}
}

void UBECharacterMovementComponent::RefreshGaitConfigs()
{
	auto AllowedRotationMode{ DesiredRotationMode };
	auto AllowedStance{ DesiredStance };
	auto AllowedGait{ DesiredGait };

	const auto& LocomotionModeConfigs{ MovementData->GetAllowedLocomotionMode(LocomotionMode) };
	const auto& RotationModeConfigs{ LocomotionModeConfigs.GetAllowedRotationMode(this, DesiredRotationMode, AllowedRotationMode) };
	const auto& StanceConfigs{ RotationModeConfigs.GetAllowedStance(DesiredStance, AllowedStance) };
	const auto& GaitConfigs{ StanceConfigs.GetAllowedGait(this, DesiredGait, AllowedGait) };

	RefreshGaitConfigs(GaitConfigs);
}

void UBECharacterMovementComponent::RefreshGaitConfigs(const FBECharacterGaitConfigs& GaitConfigs)
{
	MaxWalkSpeed			= GaitConfigs.MaxSpeed;
	MaxWalkSpeedCrouched	= GaitConfigs.MaxSpeed;
	MaxFlySpeed				= GaitConfigs.MaxSpeed;
	MaxSwimSpeed			= GaitConfigs.MaxSpeed;
	MaxCustomMovementSpeed	= GaitConfigs.MaxSpeed;

	MaxAcceleration = GaitConfigs.MaxAcceleration;

	BrakingDecelerationWalking	= GaitConfigs.BrakingDeceleration;
	BrakingDecelerationSwimming = GaitConfigs.BrakingDeceleration;
	BrakingDecelerationFlying	= GaitConfigs.BrakingDeceleration;
	BrakingDecelerationFalling	= GaitConfigs.BrakingDeceleration;

	GroundFriction = GaitConfigs.GroundFriction;

	JumpZVelocity = GaitConfigs.JumpZPower;

	AirControl = GaitConfigs.AirControl;

	RotationInterpSpeed = GaitConfigs.RotationInterpSpeed;
}


bool UBECharacterMovementComponent::TryGetMovementBaseRotationSpeed(const FBasedMovementInfo& BasedMovement, FRotator& RotationSpeed)
{
	if (!MovementBaseUtility::IsDynamicBase(BasedMovement.MovementBase))
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	const auto* BodyInstance{ BasedMovement.MovementBase->GetBodyInstance(BasedMovement.BoneName) };
	if (BodyInstance == nullptr)
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	const auto AngularVelocityVector{ BodyInstance->GetUnrealWorldAngularVelocityInRadians() };
	if (AngularVelocityVector.IsNearlyZero())
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	RotationSpeed.Roll	= FMath::RadiansToDegrees(AngularVelocityVector.X);
	RotationSpeed.Pitch = FMath::RadiansToDegrees(AngularVelocityVector.Y);
	RotationSpeed.Yaw	= FMath::RadiansToDegrees(AngularVelocityVector.Z);

	return true;
}

#pragma endregion


#pragma region Locomotion Action

void UBECharacterMovementComponent::SetLocomotionAction(const FGameplayTag& NewLocomotionAction)
{
	if (LocomotionAction != NewLocomotionAction)
	{
		LocomotionAction = NewLocomotionAction;
	}
}

#pragma endregion


#pragma region Input

FVector UBECharacterMovementComponent::ConsumeInputVector()
{
	auto InputVector{ Super::ConsumeInputVector() };

	FRotator BaseRotationSpeed;
	if (!bIgnoreBaseRotation && TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// 入力ベクトルをオフセットして、移動ベースに対する相対的な値を維持する
		InputVector = (BaseRotationSpeed * GetWorld()->GetDeltaSeconds()).RotateVector(InputVector);
	}

	return InputVector;
}

void UBECharacterMovementComponent::SetInputDirection(FVector NewInputDirection)
{
	NewInputDirection = NewInputDirection.GetSafeNormal();

	COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, InputDirection, NewInputDirection, this);
}

void UBECharacterMovementComponent::UpdateInput(float DeltaTime)
{
	if (CharacterOwner->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		SetInputDirection(GetCurrentAcceleration() / GetMaxAcceleration());
	}

	LocomotionState.bHasInput = InputDirection.SizeSquared() > UE_KINDA_SMALL_NUMBER;

	if (LocomotionState.bHasInput)
	{
		LocomotionState.InputYawAngle = UE_REAL_TO_FLOAT(UBEMovementMath::DirectionToAngleXY(InputDirection));
	}
}

#pragma endregion


#pragma region View

void UBECharacterMovementComponent::UpdateView(float DeltaTime)
{
	if (MovementBase.bHasRelativeRotation)
	{
		// Offset the rotations to keep them relative to the movement base.

		ReplicatedViewRotation.Pitch += MovementBase.DeltaRotation.Pitch;
		ReplicatedViewRotation.Yaw += MovementBase.DeltaRotation.Yaw;
		ReplicatedViewRotation.Normalize();

		ViewState.Rotation.Pitch += MovementBase.DeltaRotation.Pitch;
		ViewState.Rotation.Yaw += MovementBase.DeltaRotation.Yaw;
		ViewState.Rotation.Normalize();
	}

	ViewState.PreviousYawAngle = UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw);

	if ((CharacterOwner->IsReplicatingMovement() && CharacterOwner->GetLocalRole() >= ROLE_AutonomousProxy) || CharacterOwner->IsLocallyControlled())
	{
		SetReplicatedViewRotation(GetBECharacterOwner()->GetViewRotationSuperClass());
	}

	UpdateViewNetworkSmoothing(DeltaTime);

	ViewState.Rotation = ViewState.NetworkSmoothing.Rotation;
	ViewState.YawSpeed = FMath::Abs(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - ViewState.PreviousYawAngle)) / DeltaTime;
}

void UBECharacterMovementComponent::CorrectViewNetworkSmoothing(const FRotator& NewViewRotation)
{
	ReplicatedViewRotation = NewViewRotation;
	ReplicatedViewRotation.Normalize();

	auto& NetworkSmoothing{ ViewState.NetworkSmoothing };

	if (!NetworkSmoothing.bEnabled)
	{
		NetworkSmoothing.InitialRotation = ReplicatedViewRotation;
		NetworkSmoothing.Rotation = ReplicatedViewRotation;
		return;
	}

	const auto bListenServer{ IsNetMode(NM_ListenServer) };
	const auto NewNetworkSmoothingServerTime{ bListenServer ? GetServerLastTransformUpdateTimeStamp() : CharacterOwner->GetReplicatedServerLastTransformUpdateTimeStamp() };

	if (NewNetworkSmoothingServerTime <= 0.0f)
	{
		return;
	}

	NetworkSmoothing.InitialRotation = NetworkSmoothing.Rotation;

	// サーバー時間を使用すると、パケットラグの差異に関係なく、経過時間を知ることができる

	const auto ServerDeltaTime{ NewNetworkSmoothingServerTime - NetworkSmoothing.ServerTime };

	NetworkSmoothing.ServerTime = NewNetworkSmoothingServerTime;

	// クライアントが新しいサーバー時間より大幅に遅れたり、先に実行されたりしないようにする

	const auto MaxServerDeltaTime{ GetDefault<AGameNetworkManager>()->MaxClientSmoothingDeltaTime };
	const auto MinServerDeltaTime{ FMath::Min(MaxServerDeltaTime, bListenServer ? ListenServerNetworkSimulatedSmoothLocationTime : NetworkSimulatedSmoothLocationTime) };

	// 新しいサーバー時刻を受け取った後にどれだけ遅れることができるかを計算する

	const auto MinClientDeltaTime{ FMath::Clamp(ServerDeltaTime * 1.25f, MinServerDeltaTime, MaxServerDeltaTime) };

	NetworkSmoothing.ClientTime = FMath::Clamp(NetworkSmoothing.ClientTime, NetworkSmoothing.ServerTime - MinClientDeltaTime, NetworkSmoothing.ServerTime);

	// 新しいサーバー時間とクライアント シミュレーションの間の実際の差分を計算する

	NetworkSmoothing.Duration = NetworkSmoothing.ServerTime - NetworkSmoothing.ClientTime;
}

void UBECharacterMovementComponent::UpdateViewNetworkSmoothing(float DeltaTime)
{
	auto& NetworkSmoothing{ ViewState.NetworkSmoothing };

	if (!NetworkSmoothing.bEnabled ||
		NetworkSmoothing.ClientTime >= NetworkSmoothing.ServerTime ||
		NetworkSmoothing.Duration <= UE_SMALL_NUMBER)
	{
		NetworkSmoothing.InitialRotation = ReplicatedViewRotation;
		NetworkSmoothing.Rotation = ReplicatedViewRotation;
		return;
	}

	if (MovementBase.bHasRelativeRotation)
	{
		NetworkSmoothing.InitialRotation.Pitch += MovementBase.DeltaRotation.Pitch;
		NetworkSmoothing.InitialRotation.Yaw += MovementBase.DeltaRotation.Yaw;
		NetworkSmoothing.InitialRotation.Normalize();

		NetworkSmoothing.Rotation.Pitch += MovementBase.DeltaRotation.Pitch;
		NetworkSmoothing.Rotation.Yaw += MovementBase.DeltaRotation.Yaw;
		NetworkSmoothing.Rotation.Normalize();
	}

	NetworkSmoothing.ClientTime += DeltaTime;

	const auto InterpolationAmount{ UBEMovementMath::Clamp01(1.0f - (NetworkSmoothing.ServerTime - NetworkSmoothing.ClientTime) / NetworkSmoothing.Duration) };

	if (!FAnimWeight::IsFullWeight(InterpolationAmount))
	{
		NetworkSmoothing.Rotation = UBEMovementMath::LerpRotator(NetworkSmoothing.InitialRotation, ReplicatedViewRotation, InterpolationAmount);
		
	}
	else
	{
		NetworkSmoothing.ClientTime = NetworkSmoothing.ServerTime;
		NetworkSmoothing.Rotation = ReplicatedViewRotation;
	}
}

void UBECharacterMovementComponent::SetReplicatedViewRotation(const FRotator& NewViewRotation)
{
	if (ReplicatedViewRotation != NewViewRotation)
	{
		ReplicatedViewRotation = NewViewRotation;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedViewRotation, this);

		if (!CharacterOwner->IsReplicatingMovement() && CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetReplicatedViewRotation(ReplicatedViewRotation);
		}
	}
}

void UBECharacterMovementComponent::OnReplicated_ReplicatedViewRotation()
{
	CorrectViewNetworkSmoothing(ReplicatedViewRotation);
}

void UBECharacterMovementComponent::Server_SetReplicatedViewRotation_Implementation(const FRotator& NewViewRotation)
{
	SetReplicatedViewRotation(NewViewRotation);
}

#pragma endregion


#pragma region Locomotion State

void UBECharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UBECharacterMovementComponent::UpdateCharacterStateBeforeMovement()"), STAT_UBECharaterMovementComponent_UpdateCharacterStateBeforeMovement, STATGROUP_BEMovement);

	UpdateVisibilityBasedAnimTickOption();

	UpdateMovementBase();

	UpdateInput(DeltaSeconds);

	UpdateLocomotionEarly();

	UpdateView(DeltaSeconds);

	UpdateLocomotion(DeltaSeconds);

	UpdateLocomotionConfigs();

	UpdateGroundedRotation(DeltaSeconds);
	UpdateInAirRotation(DeltaSeconds);

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UBECharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UBECharacterMovementComponent::UpdateCharacterStateAfterMovement()"), STAT_UBECharaterMovementComponent_UpdateCharacterStateAfterMovement, STATGROUP_BEMovement);

	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

	UpdateLocomotionLate(DeltaSeconds);

	UpdateAnimInstanceMovement();
}

void UBECharacterMovementComponent::ComputeFloorDist(
	const FVector& CapsuleLocation,
	float LineDistance,
	float SweepDistance,
	FFindFloorResult& OutFloorResult,
	float SweepRadius,
	const FHitResult* DownwardSweepResult) const
{
	OutFloorResult.Clear();

	float PawnRadius, PawnHalfHeight;
	CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

	bool bSkipSweep = false;
	if (DownwardSweepResult != NULL && DownwardSweepResult->IsValidBlockingHit())
	{
		// Only if the supplied sweep was vertical and downward.
		if ((DownwardSweepResult->TraceStart.Z > DownwardSweepResult->TraceEnd.Z) &&
			(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd).SizeSquared2D() <= UE_KINDA_SMALL_NUMBER)
		{
			// Reject hits that are barely on the cusp of the radius of the capsule
			if (IsWithinEdgeTolerance(DownwardSweepResult->Location, DownwardSweepResult->ImpactPoint, PawnRadius))
			{
				// Don't try a redundant sweep, regardless of whether this sweep is usable.
				bSkipSweep = true;

				const bool bIsWalkable = IsWalkable(*DownwardSweepResult);
				const float FloorDist = (CapsuleLocation.Z - DownwardSweepResult->Location.Z);
				OutFloorResult.SetFromSweep(*DownwardSweepResult, FloorDist, bIsWalkable);

				if (bIsWalkable)
				{
					// Use the supplied downward sweep as the floor hit result.
					return;
				}
			}
		}
	}

	// We require the sweep distance to be >= the line distance, otherwise the HitResult can't be interpreted as the sweep result.
	if (SweepDistance < LineDistance)
	{
		ensure(SweepDistance >= LineDistance);
		return;
	}

	bool bBlockingHit = false;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ComputeFloorDist), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(QueryParams, ResponseParam);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

	// Sweep test
	if (!bSkipSweep && SweepDistance > 0.f && SweepRadius > 0.f)
	{
		// Use a shorter height to avoid sweeps giving weird results if we start on a surface.
		// This also allows us to adjust out of penetrations.
		const float ShrinkScale = 0.9f;
		const float ShrinkScaleOverlap = 0.1f;
		float ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScale);
		float TraceDist = SweepDistance + ShrinkHeight;
		FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(SweepRadius, PawnHalfHeight - ShrinkHeight);

		FHitResult Hit(1.f);
		bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + FVector(0.f, 0.f, -TraceDist), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);

		// TODO Start of custom ALS code block.

		const_cast<ThisClass*>(this)->SavePenetrationAdjustment(Hit);

		// TODO End of custom ALS code block.

		if (bBlockingHit)
		{
			// Reject hits adjacent to us, we only care about hits on the bottom portion of our capsule.
			// Check 2D distance to impact point, reject if within a tolerance from radius.
			if (Hit.bStartPenetrating || !IsWithinEdgeTolerance(CapsuleLocation, Hit.ImpactPoint, CapsuleShape.Capsule.Radius))
			{
				// Use a capsule with a slightly smaller radius and shorter height to avoid the adjacent object.
				// Capsule must not be nearly zero or the trace will fall back to a line trace from the start point and have the wrong length.
				CapsuleShape.Capsule.Radius = FMath::Max(0.f, CapsuleShape.Capsule.Radius - SWEEP_EDGE_REJECT_DISTANCE - UE_KINDA_SMALL_NUMBER);
				if (!CapsuleShape.IsNearlyZero())
				{
					ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScaleOverlap);
					TraceDist = SweepDistance + ShrinkHeight;
					CapsuleShape.Capsule.HalfHeight = FMath::Max(PawnHalfHeight - ShrinkHeight, CapsuleShape.Capsule.Radius);
					Hit.Reset(1.f, false);

					bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + FVector(0.f, 0.f, -TraceDist), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);
				}
			}

			// Reduce hit distance by ShrinkHeight because we shrank the capsule for the trace.
			// We allow negative distances here, because this allows us to pull out of penetrations.
			const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
			const float SweepResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

			OutFloorResult.SetFromSweep(Hit, SweepResult, false);
			if (Hit.IsValidBlockingHit() && IsWalkable(Hit))
			{
				if (SweepResult <= SweepDistance)
				{
					// Hit within test distance.
					OutFloorResult.bWalkableFloor = true;
					return;
				}
			}
		}
	}

	// Since we require a longer sweep than line trace, we don't want to run the line trace if the sweep missed everything.
	// We do however want to try a line trace if the sweep was stuck in penetration.
	if (!OutFloorResult.bBlockingHit && !OutFloorResult.HitResult.bStartPenetrating)
	{
		OutFloorResult.FloorDist = SweepDistance;
		return;
	}

	// Line trace
	if (LineDistance > 0.f)
	{
		const float ShrinkHeight = PawnHalfHeight;
		const FVector LineTraceStart = CapsuleLocation;
		const float TraceDist = LineDistance + ShrinkHeight;
		const FVector Down = FVector(0.f, 0.f, -TraceDist);
		QueryParams.TraceTag = SCENE_QUERY_STAT_NAME_ONLY(FloorLineTrace);

		FHitResult Hit(1.f);
		bBlockingHit = GetWorld()->LineTraceSingleByChannel(Hit, LineTraceStart, LineTraceStart + Down, CollisionChannel, QueryParams, ResponseParam);

		if (bBlockingHit)
		{
			if (Hit.Time > 0.f)
			{
				// Reduce hit distance by ShrinkHeight because we started the trace higher than the base.
				// We allow negative distances here, because this allows us to pull out of penetrations.
				const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
				const float LineResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

				OutFloorResult.bBlockingHit = true;
				if (LineResult <= LineDistance && IsWalkable(Hit))
				{
					OutFloorResult.SetFromLineTrace(Hit, OutFloorResult.FloorDist, LineResult, true);
					return;
				}
			}
		}
	}

	// No hits were acceptable.
	OutFloorResult.bWalkableFloor = false;
}


bool UBECharacterMovementComponent::CanAttemptJump() const
{
	if (!(IsJumpAllowed() && (IsMovingOnGround() || IsFalling()) && !LocomotionAction.IsValid()))
	{
		return false;
	}

	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->GetTagCount(TAG_Flag_MovementBlocked) > 0)
		{
			return false;
		}
		else if (AbilitySystemComponent->GetTagCount(TAG_Flag_JumpBlocked) > 0)
		{
			return false;
		}
	}
	
	return true;
}


void UBECharacterMovementComponent::ControlledCharacterMove(const FVector& InputVector, float DeltaTime)
{
	Super::ControlledCharacterMove(InputVector, DeltaTime);

	const auto* Controller{ CharacterOwner->GetController() };
	if (IsValid(Controller))
	{
		PreviousControlRotation = Controller->GetControlRotation();
	}
}

void UBECharacterMovementComponent::PhysWalking(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Walking);
		return;
	}

	// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = DeltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent* const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration.Z = 0.f;

		// Apply acceleration
		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration());
			// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
		}

		ApplyRootMotionToVelocity(timeTick);
		// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		if (IsFalling())
		{
			// Root motion could have put us into Falling.
			// No movement has taken place this movement tick so we pass on full time/past iteration count
			StartNewPhysics(remainingTime + timeTick, Iterations - 1);
			return;
		}

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsFalling())
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > UE_KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			// calculate possible alternate movement
			const FVector GravDir = FVector(0.f, 0.f, -1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if (!NewDelta.IsZero())
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// TODO Start of custom ALS code block.

						ApplyPendingPenetrationAdjustment();

						// TODO End of custom ALS code block.

						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				// TODO Start of custom ALS code block.

				ApplyPendingPenetrationAdjustment();

				// TODO End of custom ALS code block.

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if (IsSwimming())
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;
			}
		}


		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO Start of custom ALS code block.

				PrePenetrationAdjustmentVelocity = MoveVelocity;
				bPrePenetrationAdjustmentVelocityValid = true;

				// TODO End of custom ALS code block.

				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
}

void UBECharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	const auto Index{ static_cast<int32>(CustomMovementMode) };
	if (MovementData->CustomMovements.IsValidIndex(Index))
	{
		MovementData->CustomMovements[Index]->PhysMovement(this, DeltaTime, Iterations);
		return;
	}

	if (DeltaTime < MIN_TICK_TIME)
	{
		Super::PhysCustom(DeltaTime, Iterations);
		return;
	}

	Iterations += 1;
	bJustTeleported = false;

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = FVector::ZeroVector;
	}

	ApplyRootMotionToVelocity(DeltaTime);

	MoveUpdatedComponent(Velocity * DeltaTime, UpdatedComponent->GetComponentQuat(), false);

	Super::PhysCustom(DeltaTime, Iterations);
}

void UBECharacterMovementComponent::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);

	const auto* Controller{ HasValidData() ? CharacterOwner->GetController() : nullptr };

	if (IsValid(Controller) && CharacterOwner->GetLocalRole() >= ROLE_Authority &&
		PreviousControlRotation != Controller->GetControlRotation())
	{
		if (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
		{
			ServerLastTransformUpdateTimeStamp = GetPredictionData_Server_Character()->ServerAccumulatedClientTimeStamp;
		}
		else
		{
			ServerLastTransformUpdateTimeStamp = GetWorld()->GetTimeSeconds();
		}
	}
}

void UBECharacterMovementComponent::SmoothClientPosition(float DeltaTime)
{
	auto* PredictionData{ GetPredictionData_Client_Character() };
	const auto* Mesh{ HasValidData() ? CharacterOwner->GetMesh() : nullptr };

	if (PredictionData != nullptr && IsValid(Mesh) && Mesh->IsUsingAbsoluteRotation())
	{
		const auto Rotation{ Mesh->GetComponentQuat() * CharacterOwner->GetBaseRotationOffset().Inverse() };

		PredictionData->OriginalMeshRotationOffset = Rotation;
		PredictionData->MeshRotationOffset = Rotation;
		PredictionData->MeshRotationTarget = Rotation;
	}

	Super::SmoothClientPosition(DeltaTime);
}

void UBECharacterMovementComponent::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAcceleration)
{
	const auto* MoveData{ static_cast<FBECharacterNetworkMoveData*>(GetCurrentNetworkMoveData()) };
	if (MoveData != nullptr)
	{
		RotationMode	= MoveData->RotationMode;
		Stance			= MoveData->Stance;
		Gait			= MoveData->Gait;

		RefreshGaitConfigs();
	}

	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAcceleration);

	const auto* Controller{ HasValidData() ? CharacterOwner->GetController() : nullptr };

	if (IsValid(Controller) && IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
	{
		const FRotator NewControlRotation = Controller->GetControlRotation();

		CorrectViewNetworkSmoothing(NewControlRotation);

		PreviousControlRotation = NewControlRotation;
	}
}


bool UBECharacterMovementComponent::TryConsumePrePenetrationAdjustmentVelocity(FVector& OutVelocity)
{
	if (!bPrePenetrationAdjustmentVelocityValid)
	{
		OutVelocity = FVector::ZeroVector;
		return false;
	}

	OutVelocity = PrePenetrationAdjustmentVelocity;

	PrePenetrationAdjustmentVelocity = FVector::ZeroVector;
	bPrePenetrationAdjustmentVelocityValid = false;

	return true;
}

void UBECharacterMovementComponent::SavePenetrationAdjustment(const FHitResult& Hit)
{
	if (Hit.bStartPenetrating)
	{
		PendingPenetrationAdjustment = Hit.Normal * Hit.PenetrationDepth;
	}
}

void UBECharacterMovementComponent::ApplyPendingPenetrationAdjustment()
{
	if (PendingPenetrationAdjustment.IsNearlyZero())
	{
		return;
	}

	ResolvePenetration(ConstrainDirectionToPlane(PendingPenetrationAdjustment),
		CurrentFloor.HitResult, UpdatedComponent->GetComponentQuat());

	PendingPenetrationAdjustment = FVector::ZeroVector;
}


void UBECharacterMovementComponent::UpdateLocomotionEarly()
{
	if (MovementBase.bHasRelativeRotation)
	{
		// 回転をオフセットして (アクターの回転も)、動きのベースに対して相対的に保つ

		LocomotionState.TargetYawAngle = FRotator3f::NormalizeAxis(LocomotionState.TargetYawAngle + MovementBase.DeltaRotation.Yaw);
		LocomotionState.ViewRelativeTargetYawAngle = FRotator3f::NormalizeAxis(LocomotionState.ViewRelativeTargetYawAngle + MovementBase.DeltaRotation.Yaw);
		LocomotionState.SmoothTargetYawAngle = FRotator3f::NormalizeAxis(LocomotionState.SmoothTargetYawAngle + MovementBase.DeltaRotation.Yaw);

		auto NewRotation{ CharacterOwner->GetActorRotation() };
		NewRotation.Pitch += MovementBase.DeltaRotation.Pitch;
		NewRotation.Yaw += MovementBase.DeltaRotation.Yaw;
		NewRotation.Normalize();

		CharacterOwner->SetActorRotation(NewRotation);
	}

	UpdateLocomotionLocationAndRotation();

	LocomotionState.PreviousVelocity = LocomotionState.Velocity;
	LocomotionState.PreviousYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);
}

void UBECharacterMovementComponent::UpdateLocomotion(float DeltaTime)
{
	LocomotionState.Velocity = Velocity;

	// 文字が動いているかどうかは、その速度を取得することで判断する。速度は水平方向の速度の
	// したがって、垂直方向の動きは考慮されません。もし
	// キャラクターが動いている場合、最後に回転した速度を更新します。
	// この値が保存されるのはキャラクタが停止した後でも、移動の最後の方向を知っていると便利だからです。

	LocomotionState.Speed = UE_REAL_TO_FLOAT(LocomotionState.Velocity.Size2D());

	static constexpr auto HasSpeedThreshold{ 1.0f };

	LocomotionState.bHasSpeed = LocomotionState.Speed >= HasSpeedThreshold;

	if (LocomotionState.bHasSpeed)
	{
		LocomotionState.VelocityYawAngle = UE_REAL_TO_FLOAT(UBEMovementMath::DirectionToAngleXY(LocomotionState.Velocity));
	}

	if (MovementData->bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode && CharacterOwner->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		FVector DesiredVelocity;

		SetDesiredVelocityYawAngle(TryConsumePrePenetrationAdjustmentVelocity(DesiredVelocity) &&
			DesiredVelocity.Size2D() >= HasSpeedThreshold
			? UE_REAL_TO_FLOAT(UBEMovementMath::DirectionToAngleXY(DesiredVelocity))
			: LocomotionState.VelocityYawAngle);
	}

	LocomotionState.Acceleration = (LocomotionState.Velocity - LocomotionState.PreviousVelocity) / DeltaTime;

	// 速度と現在の加速度がある場合、または速度が移動速度のしきい値を超えている場合、キャラクターは移動しています。

	LocomotionState.bMoving = (LocomotionState.bHasInput && LocomotionState.bHasSpeed) ||
		LocomotionState.Speed > MovementData->MovingSpeedThreshold;
}

void UBECharacterMovementComponent::UpdateLocomotionLate(float DeltaTime)
{
	if (!LocomotionMode.IsValid() || LocomotionAction.IsValid())
	{
		UpdateLocomotionLocationAndRotation();
		UpdateTargetYawAngleUsingLocomotionRotation();
	}

	LocomotionState.YawSpeed = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw - LocomotionState.PreviousYawAngle)) / DeltaTime;
}

void UBECharacterMovementComponent::UpdateLocomotionLocationAndRotation()
{
	const auto& ActorTransform{ GetActorTransform() };

	// ネットワーク スムージングが無効な場合は、通常のアクター変換を返します。

	if (NetworkSmoothingMode == ENetworkSmoothingMode::Disabled)
	{
		LocomotionState.Location = ActorTransform.GetLocation();
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = CharacterOwner->GetActorRotation();
	}
	else if (CharacterOwner->GetMesh()->IsUsingAbsoluteRotation())
	{
		LocomotionState.Location = ActorTransform.TransformPosition(CharacterOwner->GetMesh()->GetRelativeLocation() - CharacterOwner->GetBaseTranslationOffset());
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = CharacterOwner->GetActorRotation();
	}
	else
	{
		const auto SmoothTransform{ ActorTransform *
			FTransform(CharacterOwner->GetMesh()->GetRelativeRotationCache().RotatorToQuat(CharacterOwner->GetMesh()->GetRelativeRotation()) * CharacterOwner->GetBaseRotationOffset().Inverse(),
					   CharacterOwner->GetMesh()->GetRelativeLocation() - CharacterOwner->GetBaseTranslationOffset())
		};

		LocomotionState.Location = SmoothTransform.GetLocation();
		LocomotionState.RotationQuaternion = SmoothTransform.GetRotation();
		LocomotionState.Rotation = LocomotionState.RotationQuaternion.Rotator();
	}
}

void UBECharacterMovementComponent::SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle)
{
	COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, DesiredVelocityYawAngle, NewDesiredVelocityYawAngle, this);
}

#pragma endregion 


#pragma region Rotation

void UBECharacterMovementComponent::UpdateGroundedRotation(float DeltaTime)
{
	// LocomotionAction が適応中、または OnGround ではない場合は中断する

	if (LocomotionAction.IsValid() || LocomotionMode != TAG_Status_LocomotionMode_OnGround)
	{
		return;
	}

	// RootMotion 中は回転の更新を省略して視点角度の更新だけ行う

	if (CharacterOwner->HasAnyRootMotion())
	{
		UpdateTargetYawAngleUsingLocomotionRotation();
		return;
	}

	// LocomotionState が Not Moving の場合

	if (!LocomotionState.bMoving)
	{
		ApplyRotationYawSpeed(DeltaTime);

		// RotationMode が VelocityDirection 

		if (GetRotationMode() == TAG_Status_RotationMode_VelocityDirection)
		{
			const auto TargetYawAngle{ (MovementBase.bHasRelativeLocation && !MovementBase.bHasRelativeRotation && MovementData->bInheritMovementBaseRotationInVelocityDirectionRotationMode)
										? FRotator3f::NormalizeAxis(LocomotionState.TargetYawAngle + MovementBase.DeltaRotation.Yaw)
										: LocomotionState.TargetYawAngle
			};

			static constexpr auto RotationInterpolationSpeed{ 12.0f };
			static constexpr auto TargetYawAngleRotationSpeed{ 800.0f };

			UpdateRotationExtraSmooth(TargetYawAngle, DeltaTime, RotationInterpolationSpeed, TargetYawAngleRotationSpeed);
		}

		// RotationMode が ViewDirection または Aiming

		else
		{
			static constexpr auto RotationInterpolationSpeed{ 20.0f };

			// 入力があった場合

			if (LocomotionState.bHasInput)
			{
				static constexpr auto TargetYawAngleRotationSpeed{ 1000.0f };

				UpdateRotationExtraSmooth(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw), DeltaTime, RotationInterpolationSpeed, TargetYawAngleRotationSpeed);
				return;
			}

			// 入力がなかった場合

			else
			{
				auto ViewRelativeYawAngle{ FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.Rotation.Yaw)) };

				static constexpr auto ViewRelativeYawAngleThreshold{ 70.0f };

				// キャラクターが特定の角度を超えて回転しようとしている場合

				if (FMath::Abs(ViewRelativeYawAngle) > ViewRelativeYawAngleThreshold)
				{
					if (ViewRelativeYawAngle > 180.0f - UBEMovementMath::CounterClockwiseRotationAngleThreshold)
					{
						ViewRelativeYawAngle -= 360.0f;
					}

					UpdateRotation(FRotator3f::NormalizeAxis(
						UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw + (ViewRelativeYawAngle >= 0.0f ? -ViewRelativeYawAngleThreshold : ViewRelativeYawAngleThreshold))),
							DeltaTime, RotationInterpolationSpeed);
				}

				// キャラクターが特定の角度を超えて回転し用としていない場合

				else
				{
					UpdateTargetYawAngleUsingLocomotionRotation();
				}
			}
		}
	}

	// LocomotionState が Moving の場合

	else
	{
		// RotationMode が VelocityDirection かつ入力がある

		if (GetRotationMode() == TAG_Status_RotationMode_VelocityDirection &&
			(LocomotionState.bHasInput || !LocomotionState.bRotationTowardsLastInputDirectionBlocked))
		{
			LocomotionState.bRotationTowardsLastInputDirectionBlocked = false;

			const auto TargetYawAngle{ MovementData->bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode ? DesiredVelocityYawAngle : LocomotionState.VelocityYawAngle };

			static constexpr auto TargetYawAngleRotationSpeed{ 800.0f };

			UpdateRotationExtraSmooth(TargetYawAngle, DeltaTime, CalculateRotationInterpolationSpeed(), TargetYawAngleRotationSpeed);
			return;
		}

		// RotationMode が ViewDirection または Aiming

		else
		{
			static constexpr auto RotationInterpolationSpeed{ 20.0f };
			static constexpr auto TargetYawAngleRotationSpeed{ 1000.0f };

			UpdateRotationExtraSmooth(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw), DeltaTime, RotationInterpolationSpeed, TargetYawAngleRotationSpeed);
		}
	}
}

void UBECharacterMovementComponent::UpdateInAirRotation(float DeltaTime)
{
	// LocomotionAction が適応中、または InAir ではない場合は中断する

	if (LocomotionAction.IsValid() || LocomotionMode != TAG_Status_LocomotionMode_InAir)
	{
		return;
	}

	// RotationMode が VelocityDirection

	if (GetRotationMode() == TAG_Status_RotationMode_VelocityDirection)
	{
		static constexpr auto RotationInterpolationSpeed{ 5.0f };

		// LocomotionState が Not Moving の場合

		if (!LocomotionState.bMoving)
		{
			UpdateTargetYawAngleUsingLocomotionRotation();
		}

		// LocomotionState が Moving の場合

		else
		{
			UpdateRotation(LocomotionState.VelocityYawAngle, DeltaTime, RotationInterpolationSpeed);
		}
	}

	// RotationMode が VelocityDirection または Aiming

	else
	{
		static constexpr auto RotationInterpolationSpeed{ 15.0f };

		UpdateRotation(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw), DeltaTime, RotationInterpolationSpeed);
	}
}

float UBECharacterMovementComponent::CalculateRotationInterpolationSpeed() const
{
	static constexpr auto MaxInterpolationSpeedMultiplier{ 3.0f };
	static constexpr auto ReferenceViewYawSpeed{ 300.0f };

	return RotationInterpSpeed * UBEMovementMath::LerpClamped(1.0f, MaxInterpolationSpeedMultiplier, ViewState.YawSpeed / ReferenceViewYawSpeed);
}

void UBECharacterMovementComponent::UpdateRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed)
{
	UpdateTargetYawAngle(TargetYawAngle);

	auto NewRotation{ CharacterOwner->GetActorRotation() };

	NewRotation.Yaw = UBEMovementMath::ExponentialDecayAngle(UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(NewRotation.Yaw)), TargetYawAngle, DeltaTime, RotationInterpolationSpeed);

	CharacterOwner->SetActorRotation(NewRotation);

	UpdateLocomotionLocationAndRotation();
}

void UBECharacterMovementComponent::UpdateRotationExtraSmooth(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed)
{
	LocomotionState.TargetYawAngle = TargetYawAngle;

	UpdateViewRelativeTargetYawAngle();

	LocomotionState.SmoothTargetYawAngle = UBEMovementMath::InterpolateAngleConstant(LocomotionState.SmoothTargetYawAngle, TargetYawAngle, DeltaTime, TargetYawAngleRotationSpeed);

	auto NewRotation{ CharacterOwner->GetActorRotation() };

	NewRotation.Yaw = UBEMovementMath::ExponentialDecayAngle(UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(NewRotation.Yaw)), LocomotionState.SmoothTargetYawAngle, DeltaTime, RotationInterpolationSpeed);

	CharacterOwner->SetActorRotation(NewRotation);

	UpdateLocomotionLocationAndRotation();
}

void UBECharacterMovementComponent::UpdateRotationInstant(float TargetYawAngle, ETeleportType Teleport)
{
	UpdateTargetYawAngle(TargetYawAngle);

	auto NewRotation{ CharacterOwner->GetActorRotation() };

	NewRotation.Yaw = TargetYawAngle;

	CharacterOwner->SetActorRotation(NewRotation, Teleport);

	UpdateLocomotionLocationAndRotation();
}

void UBECharacterMovementComponent::UpdateTargetYawAngleUsingLocomotionRotation()
{
	UpdateTargetYawAngle(UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw));
}

void UBECharacterMovementComponent::UpdateTargetYawAngle(float TargetYawAngle)
{
	LocomotionState.TargetYawAngle = TargetYawAngle;

	UpdateViewRelativeTargetYawAngle();

	LocomotionState.SmoothTargetYawAngle = TargetYawAngle;
}

void UBECharacterMovementComponent::UpdateViewRelativeTargetYawAngle()
{
	LocomotionState.ViewRelativeTargetYawAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.TargetYawAngle));
}

void UBECharacterMovementComponent::ApplyRotationYawSpeed(float DeltaTime)
{
	const auto DeltaYawAngle{ CharacterOwner->GetMesh()->GetAnimInstance()->GetCurveValue(UBECharacterAnimCurveNames::RotationYawSpeedCurveName()) * DeltaTime };
	
	if (FMath::Abs(DeltaYawAngle) > UE_SMALL_NUMBER)
	{
		auto NewRotation{ CharacterOwner->GetActorRotation() };

		NewRotation.Yaw += DeltaYawAngle;

		CharacterOwner->SetActorRotation(NewRotation);

		UpdateLocomotionLocationAndRotation();
		UpdateTargetYawAngleUsingLocomotionRotation();
	}
}

#pragma endregion


#pragma region Utilities

UBECharacterMovementComponent* UBECharacterMovementComponent::FindCharacterMovementComponent(const ABECharacter* Character)
{
	return (Character ? Character->FindComponentByClass<UBECharacterMovementComponent>() : nullptr);
}

#pragma endregion
