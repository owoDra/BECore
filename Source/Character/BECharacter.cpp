// Copyright owoDra

#include "BECharacter.h"

#include "Character/Component/BEPawnBasicComponent.h"
#include "Character/Component/BECharacterMovementComponent.h"
#include "Character/Component/BEPawnCameraComponent.h"
#include "Character/Movement/BECharacterMovementFragment.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "Player/BEPlayerController.h"
#include "Player/BEPlayerState.h"
#include "BELogChannels.h"
#include "GameplayTag/BETags_Status.h"

#include "AI/Navigation/NavigationTypes.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Containers/EnumAsByte.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameNetworkManager.h"
#include "GameplayTagContainer.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathSSE.h"
#include "Math/Vector.h"
#include "Misc/AssertionMacros.h"
#include "Net/UnrealNetwork.h"
#include "Templates/Casts.h"
#include "TimerManager.h"
#include "Trace/Detail/Channel.h"
#include "UObject/CoreNetTypes.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacter)

/////////////////////////////////////////////////////////////////////////////////

static FName NAME_BECharacterCollisionProfile_Capsule(TEXT("BEPawnCapsule"));
static FName NAME_BECharacterCollisionProfile_Mesh(TEXT("BEPawnMesh"));

namespace BECharacterConstants
{
	constexpr auto TeleportDistanceThresholdSquared{ FMath::Square(50.0f) };
}

/////////////////////////////////////////////////////////////////////////////////

ABECharacter::ABECharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBECharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Tickを無効化。できる限り使用しない
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Controller の回転を適応させない
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// ネットワーク関係の設定
	NetCullDistanceSquared = 900000000.0f;
	bClientCheckEncroachmentOnNetUpdate = true; // bSimGravityDisabled の更新に必須。

	// カプセルの設定
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_BECharacterCollisionProfile_Capsule);

	// TPP Mesh の設定
	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation_Direct(FRotator(0.0f, -90.0f, 0.0f));
	MeshComp->SetCollisionProfileName(NAME_BECharacterCollisionProfile_Mesh);

	// 目の位置を設定
	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;

	// Character Basic の設定
	CharacterBasic = CreateDefaultSubobject<UBEPawnBasicComponent>(TEXT("BasicComponent"));
	CharacterBasic->OnPawnInitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnPawnBasicInitialized));
	CharacterBasic->OnAbilitySystemInitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	CharacterBasic->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	BECharacterMovement = Cast<UBECharacterMovementComponent>(GetCharacterMovement());

#if WITH_EDITOR
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("Mesh") })->SetPropertyFlags(CPF_DisableEditOnInstance);
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("CapsuleComponent") })->SetPropertyFlags(CPF_DisableEditOnInstance);
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("CharacterMovement") })->SetPropertyFlags(CPF_DisableEditOnInstance);
#endif
}

#if WITH_EDITOR
bool ABECharacter::CanEditChange(const FProperty* Property) const
{
	return Super::CanEditChange(Property) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationPitch) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationYaw) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationRoll);
}
#endif

#pragma region Replication

void ABECharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
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
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, RagdollTargetLocation, Parameters);

	DOREPLIFETIME(ThisClass, MyTeamID);
}

void ABECharacter::PostNetReceiveLocationAndRotation()
{
	// この関数はシミュレートされたプロキシ上でのみ呼び出されるため、ここでロールを確認する必要はない

	const FVector PreviousLocation = GetActorLocation();

	// サーバーがレプリケートした回転を無視して Character 自体が回転を完全に制御する

	GetReplicatedMovement_Mutable().Rotation = GetActorRotation();

	Super::PostNetReceiveLocationAndRotation();

	// シミュレートされたプロキシのテレポートを検出する

	bool bTeleported = static_cast<bool>(bSimGravityDisabled);

	if (!bTeleported && !ReplicatedBasedMovement.HasRelativeLocation())
	{
		const auto NewLocation{ FRepMovement::RebaseOntoLocalOrigin(GetReplicatedMovement().Location, this) };

		bTeleported |= FVector::DistSquared(PreviousLocation, NewLocation) > BECharacterConstants::TeleportDistanceThresholdSquared;
	}

	if (bTeleported)
	{
		if (UBEAnimInstance* AnimInstance = IBEPawnMeshAssistInterface::Execute_GetTPPAnimInstance(this);)
		{
			AnimInstance->MarkTeleported();
		}
	}
}

void ABECharacter::OnRep_ReplicatedBasedMovement()
{
	// この関数はシミュレートされたプロキシ上でのみ呼び出されるため、ここでロールを確認する必要はない

	const FVector PreviousLocation = GetActorLocation();

	// サーバーがレプリケートした回転を無視して Character 自体が回転を完全に制御する

	if (ReplicatedBasedMovement.HasRelativeRotation())
	{
		FVector MovementBaseLocation;
		FQuat MovementBaseRotation;

		MovementBaseUtility::GetMovementBaseTransform(ReplicatedBasedMovement.MovementBase, ReplicatedBasedMovement.BoneName, MovementBaseLocation, MovementBaseRotation);
		ReplicatedBasedMovement.Rotation = (MovementBaseRotation.Inverse() * GetActorQuat()).Rotator();
	}
	else
	{
		ReplicatedBasedMovement.Rotation = GetActorRotation();
	}

	Super::OnRep_ReplicatedBasedMovement();

	// シミュレートされたプロキシのテレポートを検出する

	bool bTeleported = static_cast<bool>(bSimGravityDisabled);

	if (!bTeleported && BasedMovement.HasRelativeLocation())
	{
		const FVector NewLocation = GetCharacterMovement()->OldBaseLocation + GetCharacterMovement()->OldBaseQuat.RotateVector(BasedMovement.Location);

		bTeleported |= FVector::DistSquared(PreviousLocation, NewLocation) > BECharacterConstants::TeleportDistanceThresholdSquared;
	}

	if (bTeleported)
	{
		if (UBEAnimInstance* AnimInstance = IBEPawnMeshAssistInterface::Execute_GetTPPAnimInstance(this);)
		{
			AnimInstance->MarkTeleported();
		}
	}
}

#pragma endregion


#pragma region Initialize and Uninitialize

void ABECharacter::PreRegisterAllComponents()
{
	// Component が最新のデータを取得できるように
	// Component の初期化前にデフォルト値を設定する

	RotationMode	= DesiredRotationMode;
	Stance			= DesiredStance;
	Gait			= DesiredGait;

	SetReplicatedViewRotation(Super::GetViewRotation().GetNormalized());

	ViewState.NetworkSmoothing.InitialRotation = ReplicatedViewRotation;
	ViewState.NetworkSmoothing.Rotation = ReplicatedViewRotation;
	ViewState.Rotation = ReplicatedViewRotation;
	ViewState.PreviousYawAngle = UE_REAL_TO_FLOAT(ReplicatedViewRotation.Yaw);

	const auto& ActorTransform{ GetActorTransform() };

	LocomotionState.Location = ActorTransform.GetLocation();
	LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
	LocomotionState.Rotation = GetActorRotation();
	LocomotionState.PreviousYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);

	RefreshTargetYawAngleUsingLocomotionRotation();

	LocomotionState.InputYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);
	LocomotionState.VelocityYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);

	Super::PreRegisterAllComponents();
}

void ABECharacter::PostInitializeComponents()
{
	TArray<USkeletalMeshComponent*> Meshes;
	IBEPawnMeshAssistInterface::Execute_GetMeshes(this, Meshes);
	for (USkeletalMeshComponent* Mesh : Meshes)
	{
		Mesh->AddTickPrerequisiteActor(this);
	}

	BECharacterMovement->OnPhysicsRotation.AddUObject(this, &ThisClass::CharacterMovement_OnPhysicsRotation);

	Super::PostInitializeComponents();
}


void ABECharacter::NotifyControllerChanged()
{
	const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::NotifyControllerChanged();

	// Controller の TeamID をもとに Character の TeamID を設定
	if (HasAuthority() && (Controller != nullptr))
	{
		if (IBETeamAgentInterface* ControllerWithTeam = Cast<IBETeamAgentInterface>(Controller))
		{
			MyTeamID = ControllerWithTeam->GetGenericTeamId();
			ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamID);
		}
	}
}


void ABECharacter::Restart()
{
	Super::Restart();

	ApplyDesiredStance();
}

void ABECharacter::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UninitAndDestroy();
}


void ABECharacter::DisableMovementAndCollision()
{
	if (Controller)
	{
		Controller->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	UBECharacterMovementComponent* BEMoveComp = CastChecked<UBECharacterMovementComponent>(GetCharacterMovement());
	BEMoveComp->StopMovementImmediately();
	BEMoveComp->DisableMovement();
}

void ABECharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	// この Character がまだ AvatarActor として ASC に登録されている場合に初期化解除する。
	// AvatarActor 出ない場合はすでに他の Actor などから初期化解除を行っている必要がある。
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		if (BEASC->GetAvatarActor() == this)
		{
			CharacterBasic->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}


void ABECharacter::BeginPlay()
{
	ensureMsgf(!bUseControllerRotationPitch && !bUseControllerRotationYaw && !bUseControllerRotationRoll, 
				TEXT("bUseControllerRotationXXX must be turned off!"));

	Super::BeginPlay();

	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		GetCapsuleComponent()->TransformUpdated.AddWeakLambda(this, [this](USceneComponent*, const EUpdateTransformFlags, const ETeleportType TeleportType)
		{
			if (TeleportType != ETeleportType::None)
			{
				if (UBEAnimInstance* AnimInstance = IBEPawnMeshAssistInterface::Execute_GetTPPAnimInstance(this))
				{
					AnimInstance->MarkTeleported();
				}
			}
		});
	}

	RefreshUsingAbsoluteRotation();
	RefreshVisibilityBasedAnimTickOption();
}

void ABECharacter::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	RefreshUsingAbsoluteRotation();
	RefreshVisibilityBasedAnimTickOption();

	CharacterBasic->HandleControllerChanged();

	// Team の変更をリッスンする
	if (IBETeamAgentInterface* ControllerAsTeamProvider = Cast<IBETeamAgentInterface>(NewController))
	{
		MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ABECharacter::UnPossessed()
{
	AController* const OldController = Controller;

	// Team の変更のリッスンを終了する
	const FGenericTeamId OldTeamID = MyTeamID;
	if (IBETeamAgentInterface* ControllerAsTeamProvider = Cast<IBETeamAgentInterface>(OldController))
	{
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	CharacterBasic->HandleControllerChanged();

	// Controller 空所有されなくなった後の Team を設定
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ABECharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	CharacterBasic->HandleControllerChanged();
}

void ABECharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	CharacterBasic->HandlePlayerStateReplicated();
}

void ABECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	CharacterBasic->HandlePlayerInputComponentSetup();
}

#pragma endregion


#pragma region Movement Base

void ABECharacter::RefreshUsingAbsoluteRotation() const
{
	const bool bNotDedicatedServer						= !IsNetMode(NM_DedicatedServer);
	const bool bAutonomousProxyOnListenServer			= (IsNetMode(NM_ListenServer) && (GetRemoteRole() == ROLE_AutonomousProxy));
	const bool bNonLocallyControllerCharacterWithURO	= (GetMesh()->ShouldUseUpdateRateOptimizations() && !IsValid(GetInstigatorController<APlayerController>()));

	GetMesh()->SetUsingAbsoluteRotation(bNotDedicatedServer && (bAutonomousProxyOnListenServer || bNonLocallyControllerCharacterWithURO));
}

void ABECharacter::RefreshVisibilityBasedAnimTickOption() const
{
	const EVisibilityBasedAnimTickOption DefaultTickOption = GetClass()->GetDefaultObject<ABECharacter>()->GetMesh()->VisibilityBasedAnimTickOption;

	// キャラクタがリモートクライアントによって制御されているときは、サーバ上でポーズが常にチェックされていることを確認しないと
	// 何らかの問題が発生する可能性があります (ジッタなど)

	const EVisibilityBasedAnimTickOption TargetTickOption = IsNetMode(NM_Standalone) || GetLocalRole() <= ROLE_AutonomousProxy ||
		(GetRemoteRole() != ROLE_AutonomousProxy) ? EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered : EVisibilityBasedAnimTickOption::AlwaysTickPose;

	// 少なくとも、対象のtickオプションがプラグインで正しく動作するために必要でない場合は、デフォルトのtickオプションを維持する

	GetMesh()->VisibilityBasedAnimTickOption = TargetTickOption <= DefaultTickOption ? TargetTickOption : DefaultTickOption;
}

void ABECharacter::RefreshMovementBase()
{
	if (BasedMovement.MovementBase != MovementBase.Primitive ||
		BasedMovement.BoneName != MovementBase.BoneName)
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

	const FQuat PreviousRotation = MovementBase.Rotation;

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName, MovementBase.Location, MovementBase.Rotation);

	MovementBase.DeltaRotation = MovementBase.bHasRelativeLocation &&
		!MovementBase.bBaseChanged ? (MovementBase.Rotation * PreviousRotation.Inverse()).Rotator() : FRotator::ZeroRotator;
}

void ABECharacter::RefreshAnimInstanceMovement()
{
	if (!GetMesh()->bRecentlyRendered &&
		GetMesh()->VisibilityBasedAnimTickOption > EVisibilityBasedAnimTickOption::AlwaysTickPose)
	{
		GetMesh()->GetAnimInstance()->MarkPendingUpdate();
	}
}

#pragma endregion


#pragma region Locomotion Mode

void ABECharacter::SetLocomotionMode(const FGameplayTag& NewLocomotionMode)
{
	if (LocomotionMode != NewLocomotionMode)
	{
		const auto PreviousLocomotionMode{ LocomotionMode };

		LocomotionMode = NewLocomotionMode;

		OnLocomotionModeChanged(PreviousLocomotionMode);
	}
}

void ABECharacter::OnLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode)
{
	ApplyDesiredStance();

	if (LocomotionMode == TAG_Status_LocomotionMode_OnGround && PreviousLocomotionMode == TAG_Status_LocomotionMode_InAir)
	{
		// 着地後、最後に入力された方向へのキャラクターの回転をブロックします
		// 着地アニメーションの再生中に脚が螺旋状にねじれるのを防ぐ

		LocomotionState.bRotationTowardsLastInputDirectionBlocked = true;
	}

	// AbilitySystem に Tag を追加
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(PreviousLocomotionMode, 0);
		BEASC->SetLooseGameplayTagCount(LocomotionMode, 1);
	}

	K2_OnLocomotionModeChanged(PreviousLocomotionMode);
}

#pragma endregion


#pragma region Desired Rotation Mode

void ABECharacter::SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode)
{
	if (DesiredRotationMode != NewDesiredRotationMode)
	{
		DesiredRotationMode = NewDesiredRotationMode;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredRotationMode, this);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetDesiredRotationMode(DesiredRotationMode);
		}
	}
}

void ABECharacter::Server_SetDesiredRotationMode_Implementation(const FGameplayTag& NewDesiredRotationMode)
{
	SetDesiredRotationMode(NewDesiredRotationMode);
}

#pragma endregion


#pragma region Rotation Mode

void ABECharacter::RefreshRotationMode()
{
	SetRotationMode(CalculateAllowedRotationMode());
}

void ABECharacter::SetRotationMode(const FGameplayTag& NewRotationMode)
{
	BECharacterMovement->SetRotationMode(NewRotationMode);

	if (RotationMode != NewRotationMode)
	{
		const FGameplayTag PreviousRotationMode = RotationMode;

		RotationMode = NewRotationMode;

		OnRotationModeChanged(PreviousRotationMode);
	}
}

FGameplayTag ABECharacter::CalculateAllowedRotationMode() const
{
	return DesiredRotationMode;
}

void ABECharacter::OnRotationModeChanged(const FGameplayTag& PreviousRotationMode)
{
	// AbilitySystem に Tag を追加
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(PreviousRotationMode, 0);
		BEASC->SetLooseGameplayTagCount(RotationMode, 1);
	}
	
	K2_OnRotationModeChanged(PreviousRotationMode);
}

#pragma endregion


#pragma region Desired Stance

void ABECharacter::SetDesiredStance(const FGameplayTag& NewDesiredStance)
{
	if (DesiredStance != NewDesiredStance)
	{
		DesiredStance = NewDesiredStance;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredStance, this);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetDesiredStance(DesiredStance);
		}

		ApplyDesiredStance();
	}
}

void ABECharacter::ApplyDesiredStance()
{
	const FGameplayTag AllowedStance = CalculateAllowedStance();

	if (AllowedStance == TAG_Status_Stance_Standing)
	{
		UnCrouch();
	}
	else if (AllowedStance == TAG_Status_Stance_Crouching)
	{
		Crouch();
	}
}

void ABECharacter::Server_SetDesiredStance_Implementation(const FGameplayTag& NewDesiredStance)
{
	SetDesiredStance(NewDesiredStance);
}

#pragma endregion


#pragma region Stance

bool ABECharacter::CanCrouch() const
{
	return bIsCrouched || Super::CanCrouch();
}

void ABECharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	FNetworkPredictionData_Client_Character* PredictionData = GetCharacterMovement()->GetPredictionData_Client_Character();

	if (PredictionData != nullptr && GetLocalRole() <= ROLE_SimulatedProxy &&
		ScaledHalfHeightAdjust > 0.0f && IsPlayingNetworkedRootMotionMontage())
	{
		// 以下のコードでは、UCharacterMovementComponent::Crouch() の最後で行われる変更を基本的に取り消します。
		// なぜなら、ルートモーションモンタージュの再生中にしゃがむと、ネットワークスムージングが文字どおり壊れるためです。
		// これは、ルートモーションモンタージュの再生中にしゃがむと、ネットワークスムージングが文字通り中断されるためです。

		// @TODO 将来のエンジンのバージョンで、この修正の必要性を確認してください。

		PredictionData->MeshTranslationOffset.Z += ScaledHalfHeightAdjust;
		PredictionData->OriginalMeshTranslationOffset = PredictionData->MeshTranslationOffset;
	}

	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(TAG_Status_Stance_Crouching);
}

void ABECharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	FNetworkPredictionData_Client_Character* PredictionData = GetCharacterMovement()->GetPredictionData_Client_Character();

	if (PredictionData != nullptr && GetLocalRole() <= ROLE_SimulatedProxy &&
		ScaledHalfHeightAdjust > 0.0f && IsPlayingNetworkedRootMotionMontage())
	{
		// Same fix as in AAlsCharacter::OnStartCrouch().

		PredictionData->MeshTranslationOffset.Z -= ScaledHalfHeightAdjust;
		PredictionData->OriginalMeshTranslationOffset = PredictionData->MeshTranslationOffset;
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(TAG_Status_Stance_Standing);
}

void ABECharacter::SetStance(const FGameplayTag& NewStance)
{
	BECharacterMovement->SetStance(NewStance);

	if (Stance != NewStance)
	{
		const FGameplayTag PreviousStance = Stance;

		Stance = NewStance;

		OnStanceChanged(PreviousStance);
	}
}

FGameplayTag ABECharacter::CalculateAllowedStance() const
{
	if (LocomotionMode == TAG_Status_LocomotionMode_OnGround)
	{
		if (DesiredStance == TAG_Status_Stance_Standing)
		{
			UnCrouch();
		}
		else if (DesiredStance == TAG_Status_Stance_Crouching)
		{
			Crouch();
		}
	}
	else if (LocomotionMode == TAG_Status_LocomotionMode_InAir)
	{
		UnCrouch();
	}
}

void ABECharacter::OnStanceChanged(const FGameplayTag& PreviousStance)
{
	// AbilitySystem に Tag を追加
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(PreviousStance, 0);
		BEASC->SetLooseGameplayTagCount(Stance, 1);
	}

	K2_OnStanceChanged(PreviousStance);
}

#pragma endregion


#pragma region Desired Gait

void ABECharacter::SetDesiredGait(const FGameplayTag& NewDesiredGait)
{
	if (DesiredGait != NewDesiredGait)
	{
		DesiredGait = NewDesiredGait;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredGait, this);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetDesiredGait(DesiredGait);
		}
	}
}

void ABECharacter::Server_SetDesiredGait_Implementation(const FGameplayTag& NewDesiredGait)
{
	SetDesiredGait(NewDesiredGait);
}

#pragma endregion


#pragma region Gait

void ABECharacter::RefreshGait()
{
	const FGameplayTag MaxAllowedGait = CalculateMaxAllowedGait();
	
	BECharacterMovement->SetMaxAllowedGait(MaxAllowedGait);

	SetGait(CalculateActualGait(MaxAllowedGait));
}

FGameplayTag ABECharacter::CalculateMaxAllowedGait() const
{
	if (DesiredGait != TAG_Status_Gait_Sprinting)
	{
		return DesiredGait;
	}

	if (CanSprint())
	{
		return TAG_Status_Gait_Sprinting;
	}

	return TAG_Status_Gait_Running;
}

FGameplayTag ABECharacter::CalculateActualGait(const FGameplayTag& MaxAllowedGait) const
{

}

bool ABECharacter::CanSprint() const
{

}

void ABECharacter::SetGait(const FGameplayTag& NewGait)
{

}

void ABECharacter::OnGaitChanged(const FGameplayTag& PreviousGait)
{

}

#pragma endregion


#pragma region Input

void ABECharacter::RefreshInput(float DeltaTime)
{

}

void ABECharacter::SetInputDirection(FVector NewInputDirection)
{

}

#pragma endregion


#pragma region View

void ABECharacter::CorrectViewNetworkSmoothing(const FRotator& NewViewRotation)
{

}

void ABECharacter::RefreshView(float DeltaTime)
{

}

void ABECharacter::RefreshViewNetworkSmoothing(float DeltaTime)
{

}

void ABECharacter::SetReplicatedViewRotation(const FRotator& NewViewRotation)
{

}

void ABECharacter::OnReplicated_ReplicatedViewRotation()
{

}

void ABECharacter::Server_SetReplicatedViewRotation_Implementation(const FRotator& NewViewRotation)
{

}

#pragma endregion


#pragma region Locomotion State

void ABECharacter::RefreshLocomotionEarly()
{

}

void ABECharacter::RefreshLocomotion(float DeltaTime)
{

}

void ABECharacter::RefreshLocomotionLate(float DeltaTime)
{

}

void ABECharacter::RefreshLocomotionLocationAndRotation()
{

}

void ABECharacter::SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle)
{

}

#pragma endregion


#pragma region Jumping

void ABECharacter::Jump()
{

}

void ABECharacter::OnJumped_Implementation()
{

}

void ABECharacter::Multicast_OnJumpedNetworked_Implementation()
{

}

void ABECharacter::OnJumpedNetworked()
{

}

#pragma endregion


#pragma region Rotation

void ABECharacter::FaceRotation(FRotator Rotation, float DeltaTime)
{
	// 何もしない
}

void ABECharacter::RefreshGroundedRotation(float DeltaTime)
{

}

void ABECharacter::RefreshInAirRotation(float DeltaTime)
{

}

float ABECharacter::CalculateRotationInterpolationSpeed() const
{

}

void ABECharacter::RefreshGroundedMovingAimingRotation(float DeltaTime)
{

}

void ABECharacter::RefreshGroundedNotMovingAimingRotation(float DeltaTime)
{

}

void ABECharacter::RefreshInAirAimingRotation(float DeltaTime)
{

}

void ABECharacter::RefreshRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed)
{

}

void ABECharacter::RefreshRotationExtraSmooth(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed)
{

}

void ABECharacter::RefreshRotationInstant(float TargetYawAngle, ETeleportType Teleport)
{

}

void ABECharacter::RefreshTargetYawAngleUsingLocomotionRotation()
{

}

void ABECharacter::RefreshTargetYawAngle(float TargetYawAngle)
{

}

void ABECharacter::RefreshViewRelativeTargetYawAngle()
{

}

void ABECharacter::ApplyRotationYawSpeed(float DeltaTime)
{

}

#pragma endregion


#pragma region Team

void ABECharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (GetController() == nullptr)
	{
		if (HasAuthority())
		{
			const FGenericTeamId OldTeamID = MyTeamID;
			MyTeamID = NewTeamID;
			ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
		}
		else
		{
			UE_LOG(LogBETeams, Error, TEXT("You can't set the team ID on a character (%s) except on the authority"), *GetPathNameSafe(this));
		}
	}
	else
	{
		UE_LOG(LogBETeams, Error, TEXT("You can't set the team ID on a possessed character (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
	}
}

FGenericTeamId ABECharacter::GetGenericTeamId() const
{
	return MyTeamID;
}

FOnBETeamIndexChangedDelegate* ABECharacter::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

FGenericTeamId ABECharacter::DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
{
	return FGenericTeamId::NoTeam;
}

void ABECharacter::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	const FGenericTeamId MyOldTeamID = MyTeamID;
	MyTeamID = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}

void ABECharacter::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

#pragma endregion


#pragma region Meshes Assist

void ABECharacter::GetMeshes_Implementation(TArray<USkeletalMeshComponent*>& Meshes) const
{
	Meshes.Empty();
	Meshes.Emplace(GetMesh());
}

USkeletalMeshComponent* ABECharacter::GetFPPMesh_Implementation() const
{
	return nullptr;
}

USkeletalMeshComponent* ABECharacter::GetTPPMesh_Implementation() const
{
	return GetMesh();
}

void ABECharacter::GetMainAnimInstances_Implementation(TArray<UBEAnimInstance*>& Instances) const
{
	Instances.Empty();
	Instances.Emplace(Cast<UBEAnimInstance>(GetMesh()->GetAnimInstance()));
}

UBEAnimInstance* ABECharacter::GetFPPAnimInstance_Implementation() const
{
	return nullptr;
}

UBEAnimInstance* ABECharacter::GetTPPAnimInstance_Implementation() const
{
	return Cast<UBEAnimInstance>(GetMesh()->GetAnimInstance());
}

#pragma endregion


#pragma region Utilities

ABEPlayerController* ABECharacter::GetBEPlayerController() const
{
	return CastChecked<ABEPlayerController>(Controller, ECastCheckedType::NullAllowed);
}

ABEPlayerState* ABECharacter::GetBEPlayerState() const
{
	return CastChecked<ABEPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

UBEAbilitySystemComponent* ABECharacter::GetBEAbilitySystemComponent() const
{
	return CharacterBasic->GetBEAbilitySystemComponent();
}

UAbilitySystemComponent* ABECharacter::GetAbilitySystemComponent() const
{
	return CharacterBasic->GetBEAbilitySystemComponent();
}

void ABECharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool ABECharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (const UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		return BEASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool ABECharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		return BEASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool ABECharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		return BEASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

FCollisionQueryParams ABECharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

#pragma endregion
