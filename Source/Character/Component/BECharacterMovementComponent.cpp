// Copyright owoDra

#include "BECharacterMovementComponent.h"

#include "Character/Component/BEPawnBasicComponent.h"
#include "Character/Movement/BECharacterMovementFragment.h"
#include "Character/Config/BECharacterConfig_Movement.h"
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


/**
 * FBECharacterNetworkMoveData
 */
#pragma region FBECharacterNetworkMoveData

void FBECharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(Move, MoveType);

	const FBESavedMove& SavedMove = static_cast<const FBESavedMove&>(Move);

	RotationMode	= SavedMove.RotationMode;
	Stance			= SavedMove.Stance;
	MaxAllowedGait	= SavedMove.MaxAllowedGait;
}

bool FBECharacterNetworkMoveData::Serialize(UCharacterMovementComponent& Movement, FArchive& Archive, UPackageMap* Map, const ENetworkMoveType MoveType) 
{
	Super::Serialize(Movement, Archive, Map, MoveType);

	NetSerializeOptionalValue(Archive.IsSaving(), Archive, RotationMode	 , TAG_Status_RotationMode_ViewDirection.GetTag(), Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, Stance		 , TAG_Status_Stance_Standing.GetTag()			 , Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, MaxAllowedGait, TAG_Status_Gait_Walking.GetTag()				 , Map);

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

void FBESavedMove::Clear() override
{
	Super::Clear();

	RotationMode	= TAG_Status_RotationMode_ViewDirection;
	Stance			= TAG_Status_Stance_Standing;
	MaxAllowedGait	= TAG_Status_Gait_Walking;
}

void FBESavedMove::SetMoveFor(ACharacter* Character, float NewDeltaTime, const FVector& NewAcceleration, FNetworkPredictionData_Client_Character& PredictionData) override
{
	Super::SetMoveFor(Character, NewDeltaTime, NewAcceleration, PredictionData);

	const UBECharacterMovementComponent* Movement = Cast<UBECharacterMovementComponent>(Character->GetCharacterMovement());
	if (IsValid(Movement))
	{
		RotationMode	= Movement->RotationMode;
		Stance			= Movement->Stance;
		MaxAllowedGait	= Movement->MaxAllowedGait;
	}
}

bool FBESavedMove::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const
{
	const FBESavedMove* NewMove = static_cast<FBESavedMove*>(NewMovePtr.Get());

	return RotationMode == NewMove->RotationMode && Stance == NewMove->Stance && MaxAllowedGait == NewMove->MaxAllowedGait && Super::CanCombineWith(NewMovePtr, Character, MaxDelta);
}

void FBESavedMove::CombineWith(const FSavedMove_Character* PreviousMove, ACharacter* Character, APlayerController* Player, const FVector& PreviousStartLocation)
{
	// Super::CombineWith() を呼び出すと、キャラクタの回転が前の移動の回転に強制的に変更されます。
	// ABECharacter クラスで行った回転の変更が消去されるため、望ましくありません。そのため、回転を変更しないようにします、
	// Super::CombineWith() を呼び出した後、それらを復元します。

	const FRotator OriginalRotation			= PreviousMove->StartRotation;
	const FRotator OriginalRelativeRotation = PreviousMove->StartAttachRelativeRotation;

	const USceneComponent* UpdatedComponent	= Character->GetCharacterMovement()->UpdatedComponent.Get();

	const_cast<FSavedMove_Character*>(PreviousMove)->StartRotation				 = UpdatedComponent->GetComponentRotation();
	const_cast<FSavedMove_Character*>(PreviousMove)->StartAttachRelativeRotation = UpdatedComponent->GetRelativeRotation();

	Super::CombineWith(PreviousMove, Character, Player, PreviousStartLocation);

	const_cast<FSavedMove_Character*>(PreviousMove)->StartRotation				 = OriginalRotation;
	const_cast<FSavedMove_Character*>(PreviousMove)->StartAttachRelativeRotation = OriginalRelativeRotation;
}

void FBESavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UBECharacterMovementComponent* Movement = Cast<UBECharacterMovementComponent>(Character->GetCharacterMovement());
	if (IsValid(Movement))
	{
		Movement->RotationMode	 = RotationMode;
		Movement->Stance		 = Stance;
		Movement->MaxAllowedGait = MaxAllowedGait;

		Movement->RefreshGaitConfigs();
	}
}

#pragma endregion


/**
 * FBENetworkPredictionData
 */

#pragma region FBENetworkPredictionData

FBENetworkPredictionData::FBENetworkPredictionData(const UCharacterMovementComponent& Movement) {}

FSavedMovePtr FBENetworkPredictionData::AllocateNewMove()
{
	return MakeShared<FBESavedMove>();
}

#pragma endregion


//////////////////////////////////////////////
//	キャラクタームーブメント

const FName UBECharacterMovementComponent::NAME_ActorFeatureName("CharacterMovement");

UBECharacterMovementComponent* UBECharacterMovementComponent::FindCharacterMovementComponent(const ABECharacter* Character)
{
	return (Character ? Character->FindComponentByClass<UBECharacterMovementComponent>() : nullptr);
}


#pragma region Initialize Deinitialize

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

#if WITH_EDITOR

bool UBECharacterMovementComponent::CanEditChange(const FProperty* Property) const
{
	return Super::CanEditChange(Property) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, RotationRate) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerDesiredRotation) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bOrientRotationToMovement);
}

#endif

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
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetGravityScaleAttribute()).AddUObject(this, &ThisClass::HandleGravityScaleScaleChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetGroundFrictionScaleAttribute()).AddUObject(this, &ThisClass::HandleGroundFrictionScaleChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetMoveSpeedScaleAttribute()).AddUObject(this, &ThisClass::HandleMoveSpeedScaleChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetJumpPowerScaleAttribute()).AddUObject(this, &ThisClass::HandleJumpPowerScaleChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UBEMovementSet::GetAirControlScaleAttribute()).AddUObject(this, &ThisClass::HandleAirControlScaleChanged);

	InitializeGameplayTags();
}

void UBECharacterMovementComponent::UninitializeFromAbilitySystem()
{
	MovementSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void UBECharacterMovementComponent::InitializeGameplayTags()
{
	if (!CharacterOwner->HasAuthority())
	{
		return;
	}

	check(AbilitySystemComponent);
	check(MovementConfigs);

	// AbilitySystem から Movement に関する Tag を削除する

	TSet<FGameplayTag> RemovedTags;

	// LocomotionMode に関する Tag の削除

	for (const auto& KVP_LM : MovementConfigs->LocomotionModes)
	{
		if (!RemovedTags.Contains(KVP_LM.Key))
		{
			AbilitySystemComponent->SetLooseGameplayTagCount(KVP_LM.Key, 0);
			RemovedTags.Add(KVP_LM.Key);
		}

		// RotationMode に関する Tag の削除

		for (const auto& KVP_RM : KVP_LM.Value.RotationModes)
		{
			if (!RemovedTags.Contains(KVP_RM.Key))
			{
				AbilitySystemComponent->SetLooseGameplayTagCount(KVP_RM.Key, 0);
				RemovedTags.Add(KVP_RM.Key);
			}

			// Stance に関する Tag の削除

			for (const auto& KVP_St : KVP_RM.Value.Stances)
			{
				if (!RemovedTags.Contains(KVP_St.Key))
				{
					AbilitySystemComponent->SetLooseGameplayTagCount(KVP_St.Key, 0);
					RemovedTags.Add(KVP_St.Key);
				}

				// Gait に関する Tag の削除

				for (const auto& KVP_Gt : KVP_St.Value.Speeds)
				{
					if (!RemovedTags.Contains(KVP_Gt.Key))
					{
						AbilitySystemComponent->SetLooseGameplayTagCount(KVP_Gt.Key, 0);
						RemovedTags.Add(KVP_Gt.Key);
					}
				}
			}
		}
	}
	
	// 現在の Movement に関する Tag を設定する

	AbilitySystemComponent->SetLooseGameplayTagCount(LocomotionMode, 1);
	AbilitySystemComponent->SetLooseGameplayTagCount(RotationMode, 1);
	AbilitySystemComponent->SetLooseGameplayTagCount(Stance, 1);
	AbilitySystemComponent->SetLooseGameplayTagCount(Gait, 1);
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

#pragma endregion


#pragma region Gameplay Attribute

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


#pragma region Movement Basic

void UBECharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UBECharacterMovementComponent::UpdateCharacterStateBeforeMovement()"), STAT_BECharacterMovement_UpdateState, STATGROUP_BE);

	if (!IsValid(MovementConfigs))
	{
		Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
		return;
	}

	ABECharacter* BEChara = GetBECharacterOwner();

	BEChara->RefreshVisibilityBasedAnimTickOption();
	BEChara->RefreshMovementBase();

	RefreshInput(DeltaSeconds);

	RefreshLocomotionEarly();

	RefreshView(DeltaSeconds);

	RefreshRotationMode();

	RefreshLocomotion(DeltaSeconds);

	RefreshGait();

	RefreshGroundedRotation(DeltaSeconds);
	RefreshInAirRotation(DeltaSeconds);

	RefreshLocomotionLate(DeltaSeconds);

	BEChara->RefreshAnimInstanceMovement();

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UBECharacterMovementComponent::SetMovementMode(const EMovementMode NewMovementMode, const uint8 NewCustomMode)
{
	if (!bMovementModeLocked)
	{
		Super::SetMovementMode(NewMovementMode, NewCustomMode);
	}
}


void UBECharacterMovementComponent::UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation)
{
	// この関数の親実装は無視し、独自の実装を提供する
	// ABECharacter::FaceRotation() で回転の変更を無視する場合、親の実装は影響しない

	const FBasedMovementInfo& BasedMovement = CharacterOwner->GetBasedMovement();

	FVector MovementBaseLocation;
	FQuat MovementBaseRotation;

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName, MovementBaseLocation, MovementBaseRotation);

	if (!OldBaseQuat.Equals(MovementBaseRotation, UE_SMALL_NUMBER))
	{
		const FRotator DeltaRotation = (MovementBaseRotation * OldBaseQuat.Inverse()).Rotator();
		FRotator NewControlRotation  = CharacterOwner->Controller->GetControlRotation();

		NewControlRotation.Pitch += DeltaRotation.Pitch;
		NewControlRotation.Yaw   += DeltaRotation.Yaw;
		NewControlRotation.Normalize();

		CharacterOwner->Controller->SetControlRotation(NewControlRotation);
	}
}


void UBECharacterMovementComponent::CalcVelocity(const float DeltaTime, const float Friction, const bool bFluid, const float BrakingDeceleration)
{
	FRotator BaseRotationSpeed;
	if (!bIgnoreBaseRotation && TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// 移動ベースとの相対速度を保つために速度をオフセットする。
		Velocity = (BaseRotationSpeed * DeltaTime).RotateVector(Velocity);
	}

	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

float UBECharacterMovementComponent::GetMaxAcceleration() const
{
	// カーブを使用して加速度を取得します
	// これにより、各速度における動きの挙動を細かく制御することができる

	if (IsMovingOnGround() && IsValid(GaitConfigs.AccelerationAndDecelerationAndGroundFrictionCurve))
	{
		return GaitConfigs.AccelerationAndDecelerationAndGroundFrictionCurve->FloatCurves[0].Eval(CalculateGaitAmount();
	}

	return Super::GetMaxAcceleration();
}

float UBECharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	// カーブを用いて減速度を取得する
	// これにより、各速度における動きの挙動を細かく制御できる

	if (IsMovingOnGround() && IsValid(GaitConfigs.AccelerationAndDecelerationAndGroundFrictionCurve))
	{
		return GaitConfigs.AccelerationAndDecelerationAndGroundFrictionCurve->FloatCurves[1].Eval(CalculateGaitAmount();
	}

	return Super::GetMaxBrakingDeceleration();
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

	return Super::GetMaxSpeed() * MoveSpeedScale;
}

void UBECharacterMovementComponent::PhysicsRotation(const float DeltaTime)
{
	Super::PhysicsRotation(DeltaTime);

	if (HasValidData() && (bRunPhysicsWithNoController || IsValid(CharacterOwner->Controller)))
	{
		OnPhysicsRotation.Broadcast(DeltaTime);
	}
}


void UBECharacterMovementComponent::ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult, float SweepRadius, const FHitResult* DownwardSweepResult) const
{
	// @TODO UCharacterMovementComponent::ComputeFloorDist() から修正してコピー。
	// @TODO 新しいエンジン・バージョンがリリースされたら、このコードはソース・コードに合わせて更新する必要があります。

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

		const_cast<UBECharacterMovementComponent*>(this)->SavePenetrationAdjustment(Hit);

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

FNetworkPredictionData_Client* UBECharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UBECharacterMovementComponent* MutableThis = const_cast<UBECharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FBENetworkPredictionData(*this);
	}

	return ClientPredictionData;
}

void UBECharacterMovementComponent::ControlledCharacterMove(const FVector& InputVector, float DeltaTime)
{
	Super::ControlledCharacterMove(InputVector, DeltaTime);

	const AController* Controller = CharacterOwner->GetController();
	if (IsValid(Controller))
	{
		PreviousControlRotation = Controller->GetControlRotation();
	}
}

FVector UBECharacterMovementComponent::ConsumeInputVector()
{
	FVector InputVector = Super::ConsumeInputVector();

	FRotator BaseRotationSpeed;
	if (!bIgnoreBaseRotation && TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// 入力ベクトルを移動ベースに対して相対的に保つようにオフセットする。
		InputVector = (BaseRotationSpeed * GetWorld()->GetDeltaSeconds()).RotateVector(InputVector);
	}

	return InputVector;
}


void UBECharacterMovementComponent::PhysWalking(float DeltaTime, int32 Iterations)
{
	if (IsValid(GaitConfigs.AccelerationAndDecelerationAndGroundFrictionCurve))
	{
		// Get the ground friction using the movement curve. This allows for fine control over movement behavior at each speed.

		GroundFriction = GaitConfigs.AccelerationAndDecelerationAndGroundFrictionCurve->FloatCurves[2].Eval(CalculateGaitAmount()) * GroundFrictionScale;
	}
	
	// TODO Copied with modifications from UCharacterMovementComponent::PhysWalking().
	// TODO After the release of a new engine version, this code should be updated to match the source code.

	// SCOPE_CYCLE_COUNTER(STAT_CharPhysWalking);

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

void UBECharacterMovementComponent::PhysNavWalking(float DeltaTime, int32 Iterations)
{
	if (IsValid(GaitConfigs.AccelerationAndDecelerationAndGroundFrictionCurve))
	{
		// Get the ground friction using the movement curve. This allows for fine control over movement behavior at each speed.

		GroundFriction = GaitConfigs.AccelerationAndDecelerationAndGroundFrictionCurve->FloatCurves[2].Eval(CalculateGaitAmount()) * GroundFrictionScale;
	}

	Super::PhysNavWalking(DeltaTime, Iterations);
}

void UBECharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (MovementConfigs->Fragments.IsValidIndex(CustomMovementMode))
	{
		if (UBECharacterMovementFragment* Fragment = MovementConfigs->Fragments[CustomMovementMode])
		{
			Fragment->PhysMovement(this, DeltaTime, Iterations);

			return;
		}
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UBECharacterMovementComponent::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);

	// コントロールの回転が変更されたら、ServerLastTransformUpdateTimeStamp を更新する。
	// を更新する。これは、ビューネットワークスムージングが正しく動作するために必要です。

	const AController* Controller = (HasValidData() ? CharacterOwner->GetController() : nullptr);

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
	FNetworkPredictionData_Client_Character* PredictionData = GetPredictionData_Client_Character();
	const USkeletalMeshComponent* Mesh = (HasValidData() ? CharacterOwner->GetMesh() : nullptr);

	if (PredictionData != nullptr && IsValid(Mesh) && Mesh->IsUsingAbsoluteRotation())
	{
		// Super::SmoothClientPosition()を呼び出すと、メッシュの回転が変更されるため
		// 回転を変更しないようにするには、単純にメッシュの現在の回転で予測回転をオーバーライドします。

		const auto Rotation{ Mesh->GetComponentQuat() * CharacterOwner->GetBaseRotationOffset().Inverse() };

		PredictionData->OriginalMeshRotationOffset = Rotation;
		PredictionData->MeshRotationOffset = Rotation;
		PredictionData->MeshRotationTarget = Rotation;
	}

	Super::SmoothClientPosition(DeltaTime);
}

void UBECharacterMovementComponent::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAcceleration)
{
	const FBECharacterNetworkMoveData* MoveData = static_cast<FBECharacterNetworkMoveData*>(GetCurrentNetworkMoveData());
	if (MoveData != nullptr)
	{
		RotationMode	= MoveData->RotationMode;
		Stance			= MoveData->Stance;
		MaxAllowedGait  = MoveData->MaxAllowedGait;

		RefreshGaitConfigs();
	}

	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAcceleration);

	// Process view network smoothing on the listen server.

	const AController* Controller = (HasValidData() ? CharacterOwner->GetController() : nullptr);

	if (IsValid(Controller) && IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
	{
		const FRotator NewControlRotation = Controller->GetControlRotation();

		ABECharacter* Character = GetBECharacterOwner();
		if (IsValid(Character))
		{
			Character->CorrectViewNetworkSmoothing(NewControlRotation);
		}

		PreviousControlRotation = NewControlRotation;
	}
}

#pragma endregion


#pragma region Movement Advanced

bool UBECharacterMovementComponent::TryGetMovementBaseRotationSpeed(const FBasedMovementInfo& BasedMovement, FRotator& RotationSpeed)
{
	if (!MovementBaseUtility::IsDynamicBase(BasedMovement.MovementBase))
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	const FBodyInstance* BodyInstance = BasedMovement.MovementBase->GetBodyInstance(BasedMovement.BoneName);
	if (BodyInstance == nullptr)
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	const FVector AngularVelocityVector = BodyInstance->GetUnrealWorldAngularVelocityInRadians();
	if (AngularVelocityVector.IsNearlyZero())
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	RotationSpeed.Roll = FMath::RadiansToDegrees(AngularVelocityVector.X);
	RotationSpeed.Pitch = FMath::RadiansToDegrees(AngularVelocityVector.Y);
	RotationSpeed.Yaw = FMath::RadiansToDegrees(AngularVelocityVector.Z);

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

void UBECharacterMovementComponent::SetMovementModeLocked(bool bNewMovementModeLocked)
{
	bMovementModeLocked = bNewMovementModeLocked;
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

#pragma endregion


#pragma region Locomotion Mode

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
		if (MovementConfigs->Fragments.IsValidIndex(PreviousCustomMode))
		{
			if (UBECharacterMovementFragment* Fragment = MovementConfigs->Fragments[PreviousCustomMode])
			{
				Fragment->OnMovementStart(this);

				SetLocomotionMode(Fragment->GetLocomotionMode());
			}
		}
	}

	// 前回の MovementMode が Custom

	if (PreviousMovementMode == MOVE_Custom)
	{
		if (MovementConfigs->Fragments.IsValidIndex(PreviousCustomMode))
		{
			if (UBECharacterMovementFragment* Fragment = MovementConfigs->Fragments[PreviousCustomMode])
			{
				Fragment->OnMovementEnd(this);
			}
		}
	}
}

#pragma endregion


#pragma region Rotation Mode

void UBECharacterMovementComponent::SetRotationMode(const FGameplayTag& NewRotationMode)
{
	if (RotationMode != NewRotationMode)
	{
		RotationMode = NewRotationMode;

		RefreshGaitConfigs();
	}
}

#pragma endregion


#pragma region Stance

void UBECharacterMovementComponent::SetStance(const FGameplayTag& NewStance)
{
	if (Stance != NewStance)
	{
		Stance = NewStance;

		RefreshGaitConfigs();
	}
}

#pragma endregion


#pragma region Gait

void UBECharacterMovementComponent::SetMaxAllowedGait(const FGameplayTag& NewMaxAllowedGait)
{
	if (MaxAllowedGait != NewMaxAllowedGait)
	{
		MaxAllowedGait = NewMaxAllowedGait;

		RefreshMaxWalkSpeed();
	}
}

float UBECharacterMovementComponent::CalculateGaitAmount() const
{
	// キャラクタの現在の速度を、設定された 0 から 3 までの移動速度にマップします、
	// ここで、0 は停止、1 は歩行、2 は走行、3 は疾走です。これにより
	// 移動速度を変化させても、マッピングされた範囲を計算で使用することで一貫した結果を得ることができます。

	const auto Speed{ UE_REAL_TO_FLOAT(Velocity.Size2D()) };

	FVector2f GaitAmount = { 0.0, 1.0 };
	FVector2f SpeedRange = { 0.0, 0.0 };

	for (const auto& KVP : GaitConfigs.Speeds)
	{
		SpeedRange.X = SpeedRange.Y;
		SpeedRange.Y = KVP.Value;

		if (Speed <= SpeedRange.Y)
		{
			return FMath::GetMappedRangeValueClamped(SpeedRange, GaitAmount, Speed);
		}

		GaitAmount += { 1.0, 1.0 };
	}

	return GaitAmount.Y;
}

void UBECharacterMovementComponent::RefreshGaitConfigs()
{
	if (IsValid(MovementConfigs))
	{
		const FMovementLocomotionModeConfigs* LocomotionModeConfigs = MovementConfigs->LocomotionModes.Find(LocomotionMode);
		const FMovementStanceConfigs* StanceConfigs = LocomotionModeConfigs ? LocomotionModeConfigs->RotationModes.Find(RotationMode) : nullptr;
		const FMovementGaitConfigs* NewGaitConfigs = StanceConfigs ? StanceConfigs->Stances.Find(Stance) : nullptr;

		GaitConfigs = NewGaitConfigs ? *NewGaitConfigs : FMovementGaitConfigs();
	}

	RefreshMaxWalkSpeed();
}

void UBECharacterMovementComponent::RefreshMaxWalkSpeed()
{
	MaxWalkSpeed = GaitConfigs.GetSpeedForGait(MaxAllowedGait);
	MaxWalkSpeedCrouched = MaxWalkSpeed;
}

#pragma endregion


#pragma region Movement Input

void UBECharacterMovementComponent::RefreshInput(float DeltaTime)
{
	if (CharacterOwner->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		GetBECharacterOwner()->SetInputDirection(GetCurrentAcceleration() / GetMaxAcceleration());
	}

	const FVector_NetQuantizeNormal Direction = GetBECharacterOwner()->InputDirection;

	LocomotionState.bHasInput = Direction.SizeSquared() > UE_KINDA_SMALL_NUMBER;

	if (LocomotionState.bHasInput)
	{
		LocomotionState.InputYawAngle = UE_REAL_TO_FLOAT(FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X)));
	}
}

#pragma endregion


#pragma region Movement Locomotion State

void UBECharacterMovementComponent::RefreshLocomotionLocationAndRotation()
{
	const auto& ActorTransform{ GetActorTransform() };

	// ネットワーク スムージングが無効な場合は、通常のアクター変換を返します。

	if (NetworkSmoothingMode == ENetworkSmoothingMode::Disabled)
	{
		LocomotionState.Location = ActorTransform.GetLocation();
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = GetActorRotation();
	}
	else if (CharacterOwner->GetMesh()->IsUsingAbsoluteRotation())
	{
		LocomotionState.Location = ActorTransform.TransformPosition(CharacterOwner->GetMesh()->GetRelativeLocation() - CharacterOwner->GetBaseTranslationOffset());
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = CharacterOwner->GetActorRotation();
	}
	else
	{
		const auto SmoothTransform{
			ActorTransform * FTransform{
				GetMesh()->GetRelativeRotationCache().RotatorToQuat(GetMesh()->GetRelativeRotation()) * GetBaseRotationOffset().Inverse(),
				GetMesh()->GetRelativeLocation() - GetBaseTranslationOffset()
			}
		};

		LocomotionState.Location = SmoothTransform.GetLocation();
		LocomotionState.RotationQuaternion = SmoothTransform.GetRotation();
		LocomotionState.Rotation = LocomotionState.RotationQuaternion.Rotator();
	}
}

void UBECharacterMovementComponent::RefreshLocomotionEarly()
{

}

void UBECharacterMovementComponent::RefreshLocomotion(float DeltaTime)
{

}

void UBECharacterMovementComponent::RefreshLocomotionLate(float DeltaTime)
{

}

#pragma endregion


#pragma region Movement View

void UBECharacterMovementComponent::RefreshView(float DeltaTime)
{

}

void UBECharacterMovementComponent::RefreshViewNetworkSmoothing(float DeltaTime)
{

}

#pragma endregion


#pragma region Rotation

void UBECharacterMovementComponent::CharacterMovement_OnPhysicsRotation(float DeltaTime)
{

}


void UBECharacterMovementComponent::RefreshGroundedRotation(float DeltaTime)
{

}

bool UBECharacterMovementComponent::RefreshCustomGroundedMovingRotation(float DeltaTime)
{

}

bool UBECharacterMovementComponent::RefreshCustomGroundedNotMovingRotation(float DeltaTime)
{

}

void UBECharacterMovementComponent::RefreshGroundedMovingAimingRotation(float DeltaTime)
{

}

void UBECharacterMovementComponent::RefreshGroundedNotMovingAimingRotation(float DeltaTime)
{

}

float UBECharacterMovementComponent::CalculateRotationInterpolationSpeed() const
{

}


void UBECharacterMovementComponent::ApplyRotationYawSpeed(float DeltaTime)
{

}

void UBECharacterMovementComponent::RefreshInAirRotation(float DeltaTime)
{

}

bool UBECharacterMovementComponent::RefreshCustomInAirRotation(float DeltaTime)
{

}

void UBECharacterMovementComponent::RefreshInAirAimingRotation(float DeltaTime)
{

}

void UBECharacterMovementComponent::RefreshRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed)
{

}

void UBECharacterMovementComponent::RefreshRotationExtraSmooth(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed)
{

}

void UBECharacterMovementComponent::RefreshRotationInstant(float TargetYawAngle, ETeleportType Teleport = ETeleportType::None)
{

}

void UBECharacterMovementComponent::RefreshTargetYawAngleUsingLocomotionRotation()
{

}

void UBECharacterMovementComponent::RefreshTargetYawAngle(float TargetYawAngle)
{

}

void UBECharacterMovementComponent::RefreshViewRelativeTargetYawAngle()
{

}

#pragma endregion
