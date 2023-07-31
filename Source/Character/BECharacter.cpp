// Copyright owoDra

#include "BECharacter.h"

#include "Character/Component/BEPawnBasicComponent.h"
#include "Character/Component/BECharacterMovementComponent.h"
#include "Character/Component/BEPawnCameraComponent.h"
#include "Character/Animation/BECharacterAnimInstance.h"
#include "Character/Movement/BEMovementMathLibrary.h"
#include "Character/Movement/State/BEViewState.h"
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

static const FName NAME_BECharacterCollisionProfile_Capsule(TEXT("BEPawnCapsule"));
static const FName NAME_BECharacterCollisionProfile_Mesh(TEXT("BEPawnMesh"));

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

	DOREPLIFETIME(ThisClass, MyTeamID);
}

void ABECharacter::PostNetReceiveLocationAndRotation()
{
	// この関数はシミュレートされたプロキシ上でのみ呼び出されるため、ここでロールを確認する必要はない

	const auto PreviousLocation{ GetActorLocation() };

	// サーバーがレプリケートした回転を無視して Character 自体が回転を完全に制御する

	GetReplicatedMovement_Mutable().Rotation = GetActorRotation();

	Super::PostNetReceiveLocationAndRotation();

	// シミュレートされたプロキシのテレポートを検出する

	auto bTeleported{ static_cast<bool>(bSimGravityDisabled) };

	if (!bTeleported && !ReplicatedBasedMovement.HasRelativeLocation())
	{
		const auto NewLocation{ FRepMovement::RebaseOntoLocalOrigin(GetReplicatedMovement().Location, this) };

		bTeleported |= FVector::DistSquared(PreviousLocation, NewLocation) > UBEMovementMath::TeleportDistanceThresholdSquared;
	}

	if (bTeleported)
	{
		UBEAnimInstance* BEAnimIns{ nullptr };

		IBEPawnMeshAssistInterface::Execute_GetMainAnimInstance(this, BEAnimIns);

		if (UBECharacterAnimInstance* BECharaAnimIns = Cast<UBECharacterAnimInstance>(BEAnimIns))
		{
			BECharaAnimIns->MarkPendingUpdate();
		}
	}
}

void ABECharacter::OnRep_ReplicatedBasedMovement()
{
	// この関数はシミュレートされたプロキシ上でのみ呼び出されるため、ここでロールを確認する必要はない

	const auto PreviousLocation{ GetActorLocation() };

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

	auto bTeleported{ static_cast<bool>(bSimGravityDisabled) };

	if (!bTeleported && BasedMovement.HasRelativeLocation())
	{
		const auto NewLocation{ GetCharacterMovement()->OldBaseLocation + GetCharacterMovement()->OldBaseQuat.RotateVector(BasedMovement.Location) };

		bTeleported |= FVector::DistSquared(PreviousLocation, NewLocation) > UBEMovementMath::TeleportDistanceThresholdSquared;
	}

	if (bTeleported)
	{
		UBEAnimInstance* BEAnimIns{ nullptr };

		IBEPawnMeshAssistInterface::Execute_GetMainAnimInstance(this, BEAnimIns);

		if (UBECharacterAnimInstance* BECharaAnimIns = Cast<UBECharacterAnimInstance>(BEAnimIns))
		{
			BECharaAnimIns->MarkPendingUpdate();
		}
	}
}

#pragma endregion


#pragma region Initialize and Uninitialize

void ABECharacter::PostInitializeComponents()
{
	TArray<USkeletalMeshComponent*> Meshes;
	IBEPawnMeshAssistInterface::Execute_GetMeshes(this, Meshes);
	for (USkeletalMeshComponent* Each : Meshes)
	{
		Each->AddTickPrerequisiteActor(this);
	}

	Super::PostInitializeComponents();
}


void ABECharacter::NotifyControllerChanged()
{
	const auto OldTeamId{ GetGenericTeamId() };

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

	auto* CapsuleComp{ GetCapsuleComponent() };
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	auto* BEMoveComp{ CastChecked<UBECharacterMovementComponent>(GetCharacterMovement()) };
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
	ensureMsgf(!bUseControllerRotationPitch && !bUseControllerRotationYaw && !bUseControllerRotationRoll, TEXT("bUseControllerRotationXXX must be turned off!"));

	Super::BeginPlay();

	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		GetCapsuleComponent()->TransformUpdated.AddWeakLambda(this, [this](USceneComponent*, const EUpdateTransformFlags, const ETeleportType TeleportType)
		{
			if (TeleportType != ETeleportType::None)
			{
				UBEAnimInstance* BEAnimIns{ nullptr };

				IBEPawnMeshAssistInterface::Execute_GetMainAnimInstance(this, BEAnimIns);

				if (UBECharacterAnimInstance* BECharaAnimIns = Cast<UBECharacterAnimInstance>(BEAnimIns))
				{
					BECharaAnimIns->MarkPendingUpdate();
				}
			}
		});
	}

	GetBECharacterMovement()->UpdateUsingAbsoluteRotation();
	GetBECharacterMovement()->UpdateVisibilityBasedAnimTickOption();
}

void ABECharacter::PossessedBy(AController* NewController)
{
	const auto OldTeamID{ MyTeamID };

	Super::PossessedBy(NewController);

	GetBECharacterMovement()->UpdateUsingAbsoluteRotation();
	GetBECharacterMovement()->UpdateVisibilityBasedAnimTickOption();

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
	AController* OldController{ Controller };

	// Team の変更のリッスンを終了する
	const auto OldTeamID{ MyTeamID };
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


#pragma region Stance

bool ABECharacter::CanJumpInternal_Implementation() const
{
	return JumpIsAllowedInternal();
}

bool ABECharacter::CanCrouch() const
{
	return bIsCrouched || Super::CanCrouch();
}

void ABECharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	auto* PredictionData{ GetCharacterMovement()->GetPredictionData_Client_Character() };

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

	BECharacterMovement->SetStanceIndex(BECharacterMovement->GetDesiredStanceIndex());
}

void ABECharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	auto* PredictionData{ GetCharacterMovement()->GetPredictionData_Client_Character() };

	if (PredictionData != nullptr && GetLocalRole() <= ROLE_SimulatedProxy &&
		ScaledHalfHeightAdjust > 0.0f && IsPlayingNetworkedRootMotionMontage())
	{
		// Same fix as in AAlsCharacter::OnStartCrouch().

		PredictionData->MeshTranslationOffset.Z -= ScaledHalfHeightAdjust;
		PredictionData->OriginalMeshTranslationOffset = PredictionData->MeshTranslationOffset;
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	BECharacterMovement->SetStanceIndex(BECharacterMovement->GetDesiredStanceIndex());
}

#pragma endregion


#pragma region View

FRotator ABECharacter::GetViewRotation() const
{
	return BECharacterMovement->GetViewState().Rotation;
}

FRotator ABECharacter::GetViewRotationSuperClass() const
{
	if (Controller != nullptr)
	{
		return Controller->GetControlRotation();
	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		// check if being spectated
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (PlayerController &&
				PlayerController->PlayerCameraManager &&
				PlayerController->PlayerCameraManager->GetViewTargetPawn() == this)
			{
				return PlayerController->BlendedTargetViewRotation;
			}
		}
	}

	return GetActorRotation();
}


#pragma endregion


#pragma region Jump

void ABECharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	if (IsLocallyControlled())
	{
		OnJumpedNetworked();
	}

	if (GetLocalRole() >= ROLE_Authority)
	{
		Multicast_OnJumpedNetworked();
	}
}

void ABECharacter::Multicast_OnJumpedNetworked_Implementation()
{
	if (!IsLocallyControlled())
	{
		OnJumpedNetworked();
	}
}

void ABECharacter::OnJumpedNetworked()
{
	UBEAnimInstance* BEAnimIns{ nullptr };

	IBEPawnMeshAssistInterface::Execute_GetMainAnimInstance(this, BEAnimIns);

	if (UBECharacterAnimInstance* BECharaAnimIns = Cast<UBECharacterAnimInstance>(BEAnimIns))
	{
		BECharaAnimIns->Jump();
	}
}

#pragma endregion


#pragma region Team

void ABECharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (GetController() == nullptr)
	{
		if (HasAuthority())
		{
			const auto OldTeamID{ MyTeamID };
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
	const auto MyOldTeamID{ MyTeamID };
	MyTeamID = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}

void ABECharacter::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

#pragma endregion


#pragma region Meshes Assist

void ABECharacter::GetMeshes_Implementation(TArray<USkeletalMeshComponent*>& OutMeshes) const
{
	OutMeshes.Empty();
	OutMeshes.Emplace(GetMesh());
}

void ABECharacter::GetFPPMeshes_Implementation(TArray<USkeletalMeshComponent*>& OutMeshes) const
{
	OutMeshes.Empty();
}

void ABECharacter::GetTPPMeshes_Implementation(TArray<USkeletalMeshComponent*>& OutMeshes) const
{
	OutMeshes.Empty();
	OutMeshes.Emplace(GetMesh());
}

void ABECharacter::GetFPPFirstMesh_Implementation(USkeletalMeshComponent*& OutMesh) const
{
	OutMesh = nullptr;
}

void ABECharacter::GetTPPFirstMesh_Implementation(USkeletalMeshComponent*& OutMesh) const
{
	OutMesh = GetMesh();
}


void ABECharacter::GetMainAnimInstance_Implementation(UBEAnimInstance*& OutInstance) const
{
	OutInstance = Cast<UBEAnimInstance>(GetMesh()->GetAnimInstance());
}

void ABECharacter::GetSubAnimInstances_Implementation(TArray<UAnimInstance*>& OutInstances) const
{
	OutInstances.Empty();
}

void ABECharacter::GetTPPAnimInstance_Implementation(UAnimInstance*& OutInstance) const
{
	OutInstance = GetMesh()->GetAnimInstance();
}

void ABECharacter::GetFPPAnimInstance_Implementation(UAnimInstance*& OutInstance) const
{
	OutInstance = nullptr;
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

UBECharacterAnimInstance* ABECharacter::GetBEMainTPPAnimInstance() const
{
	return CastChecked<UBECharacterAnimInstance>(GetMesh()->GetAnimInstance(), ECastCheckedType::NullAllowed);
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

#pragma endregion
