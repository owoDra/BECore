// Copyright Eigi Chin

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

/////////////////////////////////////////////////////////////////////////////////

ABECharacter::ABECharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBECharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Tickを無効化。できる限り使用しない
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	NetCullDistanceSquared = 900000000.0f;

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_BECharacterCollisionProfile_Capsule);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MeshComp->SetCollisionProfileName(NAME_BECharacterCollisionProfile_Mesh);

	CharacterBasicComponent = CreateDefaultSubobject<UBEPawnBasicComponent>(TEXT("BasicComponent"));
	CharacterBasicComponent->OnPawnInitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnPawnBasicInitialized));
	CharacterBasicComponent->OnAbilitySystemInitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	CharacterBasicComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));


	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

void ABECharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);

	DOREPLIFETIME_CONDITION(ThisClass, bIsSprinting, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, bIsTargeting, COND_SimulatedOnly);

	DOREPLIFETIME(ThisClass, MyTeamID);
}


void ABECharacter::OnRep_ReplicatedAcceleration()
{
	if (UBECharacterMovementComponent* BEMovementComponent = Cast<UBECharacterMovementComponent>(GetCharacterMovement()))
	{
		// 圧縮された Acceleration を展開
		// [0, 255] -> [0, MaxAccel]
		// [0, 255] -> [0, 2PI]
		const double MaxAccel = BEMovementComponent->MaxAcceleration;
		const double AccelXYMagnitude = double(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; 
		const double AccelXYRadians = double(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;     

		// [-127, 127] -> [-MaxAccel, MaxAccel]
		FVector UnpackedAcceleration = FVector::ZeroVector;
		FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
		UnpackedAcceleration.Z = double(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; 
		BEMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
	}
}

void ABECharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// XY に方向とその大きさ、Z に直接大きさを圧縮する
		const double MaxAccel = MovementComponent->MaxAcceleration;
		const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
		double AccelXYRadians, AccelXYMagnitude;
		FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

		// [0, 2PI] -> [0, 255]
		// [0, MaxAccel] -> [0, 255]
		// [-MaxAccel, MaxAccel] -> [-127, 127]
		ReplicatedAcceleration.AccelXYRadians = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);     
		ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);	
		ReplicatedAcceleration.AccelZ = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);   
	}
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
			CharacterBasicComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}


void ABECharacter::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	CharacterBasicComponent->HandleControllerChanged();

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

	CharacterBasicComponent->HandleControllerChanged();

	// Controller 空所有されなくなった後の Team を設定
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ABECharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	CharacterBasicComponent->HandleControllerChanged();
}

void ABECharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	CharacterBasicComponent->HandlePlayerStateReplicated();
}

void ABECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	CharacterBasicComponent->HandlePlayerInputComponentSetup();
}


void ABECharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Crouching, 1);
	}

	if (UpdateMeshPositionWhenCrouch)
	{
		Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	}
	else
	{
		RecalculateBaseEyeHeight();

		K2_OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	}
}

void ABECharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Crouching, 0);
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}


void ABECharacter::ResetJumpState()
{
	bWasJumping = false;
	JumpKeyHoldTime = 0.0f;
	JumpForceTimeRemaining = 0.0f;

	if (GetCharacterMovement() && !GetCharacterMovement()->IsFalling())
	{
		JumpCurrentCount = 0;
		JumpCurrentCountPreJump = 0;
	}
}

void ABECharacter::ClearJumpInput(float DeltaTime)
{
	if (bPressedJump)
	{
		JumpKeyHoldTime += DeltaTime;

		// @Memo: bPressedJump = false を削除し入力の開始と終了が分かるようにする
	}
	else
	{
		JumpForceTimeRemaining = 0.0f;
		bWasJumping = false;
	}
}

bool ABECharacter::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}


void ABECharacter::OnRep_IsSprinting()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			if (bIsSprinting)
			{
				BECMC->bWantsToSprint = true;
				BECMC->Sprint(true);
			}
			else
			{
				BECMC->bWantsToSprint = false;
				BECMC->UnSprint(true);
			}
		}
	}
}

void ABECharacter::Sprint()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			BECMC->bWantsToSprint = true;
		}
	}
}

void ABECharacter::UnSprint()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			BECMC->bWantsToSprint = false;
		}
	}
}

void ABECharacter::OnStartSprint()
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Sprinting, 1);
	}

	OnSprintChanged.Broadcast(true);

	K2_OnStartSprint();
}

void ABECharacter::OnEndSprint()
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Sprinting, 0);
	}

	OnSprintChanged.Broadcast(false);

	K2_OnEndSprint();
}


void ABECharacter::OnRep_IsTargeting()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			if (bIsTargeting)
			{
				BECMC->bWantsToTarget = true;
				BECMC->Target(true);
			}
			else
			{
				BECMC->bWantsToTarget = false;
				BECMC->UnTarget(true);
			}
		}
	}
}

void ABECharacter::Target()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			BECMC->bWantsToTarget = true;
		}
	}
}

void ABECharacter::UnTarget()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			BECMC->bWantsToTarget = false;
		}
	}
}

void ABECharacter::OnStartTarget()
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Targeting, 1);
	}

	OnTargetChanged.Broadcast(true);

	K2_OnStartTarget();
}

void ABECharacter::OnEndTarget()
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Targeting, 0);
	}

	OnTargetChanged.Broadcast(false);

	K2_OnEndTarget();
}


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
	return CharacterBasicComponent->GetBEAbilitySystemComponent();
}

UAbilitySystemComponent* ABECharacter::GetAbilitySystemComponent() const
{
	return CharacterBasicComponent->GetBEAbilitySystemComponent();
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
