// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BECharacter.h"

#include "Character/BEPawnExtensionComponent.h"
#include "Character/BEHealthComponent.h"
#include "BECharacterMovementComponent.h"
#include "Player/BEPlayerController.h"
#include "Player/BEPlayerState.h"
#include "System/BESignificanceManager.h"
#include "Camera/BECameraComponent.h"
#include "Ability/BEAbilitySystemComponent.h"
#include "BELogChannels.h"
#include "BEGameplayTags.h"

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
#include "SignificanceManager.h"
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
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
	MeshComp->SetCollisionProfileName(NAME_BECharacterCollisionProfile_Mesh);

	PawnExtComponent = CreateDefaultSubobject<UBEPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	HealthComponent = CreateDefaultSubobject<UBEHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	CameraComponent = CreateDefaultSubobject<UBECameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

void ABECharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ABECharacter::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();

	const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
	if (bRegisterWithSignificanceManager)
	{
		if (UBESignificanceManager* SignificanceManager = USignificanceManager::Get<UBESignificanceManager>(World))
		{
//@TODO: SignificanceManager->RegisterObject(this, (EFortSignificanceType)SignificanceType);
		}
	}
}

void ABECharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UWorld* World = GetWorld();

	const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
	if (bRegisterWithSignificanceManager)
	{
		if (UBESignificanceManager* SignificanceManager = USignificanceManager::Get<UBESignificanceManager>(World))
		{
			SignificanceManager->UnregisterObject(this);
		}
	}
}

void ABECharacter::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UninitAndDestroy();
}

void ABECharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);

	DOREPLIFETIME_CONDITION(ThisClass, bIsRunning, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, bIsAiming, COND_SimulatedOnly);

	DOREPLIFETIME(ThisClass, MyTeamID);
}

void ABECharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// Compress Acceleration: XY components as direction + magnitude, Z component as direct value
		const double MaxAccel = MovementComponent->MaxAcceleration;
		const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
		double AccelXYRadians, AccelXYMagnitude;
		FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

		ReplicatedAcceleration.AccelXYRadians   = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);     // [0, 2PI] -> [0, 255]
		ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);	// [0, MaxAccel] -> [0, 255]
		ReplicatedAcceleration.AccelZ           = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);   // [-MaxAccel, MaxAccel] -> [-127, 127]
	}
}

void ABECharacter::NotifyControllerChanged()
{
	const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::NotifyControllerChanged();

	// Update our team ID based on the controller
	if (HasAuthority() && (Controller != nullptr))
	{
		if (IBETeamAgentInterface* ControllerWithTeam = Cast<IBETeamAgentInterface>(Controller))
		{
			MyTeamID = ControllerWithTeam->GetGenericTeamId();
			ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamID);
		}
	}
}

void ABECharacter::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	PawnExtComponent->HandleControllerChanged();

	// Grab the current team ID and listen for future changes
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

	// Stop listening for changes from the old controller
	const FGenericTeamId OldTeamID = MyTeamID;
	if (IBETeamAgentInterface* ControllerAsTeamProvider = Cast<IBETeamAgentInterface>(OldController))
	{
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	PawnExtComponent->HandleControllerChanged();

	// Determine what the new team ID should be afterwards
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ABECharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->HandleControllerChanged();
}

void ABECharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->HandlePlayerStateReplicated();
}

void ABECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtComponent->SetupPlayerInputComponent();
}

void ABECharacter::OnAbilitySystemInitialized()
{
	UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent();
	check(BEASC);

	HealthComponent->InitializeWithAbilitySystem(BEASC);

	UBECharacterMovementComponent* BECMC = CastChecked<UBECharacterMovementComponent>(GetCharacterMovement());
	BECMC->InitializeWithAbilitySystem(BEASC);

	InitializeGameplayTags();
}

void ABECharacter::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();

	UBECharacterMovementComponent* BECMC = CastChecked<UBECharacterMovementComponent>(GetCharacterMovement());
	BECMC->UninitializeFromAbilitySystem();
}

void ABECharacter::InitializeGameplayTags()
{
	// Clear tags that may be lingering on the ability system from the previous pawn.
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		const FBEGameplayTags& GameplayTags = FBEGameplayTags::Get();

		for (const TPair<uint8, FGameplayTag>& TagMapping : GameplayTags.MovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				BEASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		for (const TPair<uint8, FGameplayTag>& TagMapping : GameplayTags.CustomMovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				BEASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		UBECharacterMovementComponent* BEMoveComp = CastChecked<UBECharacterMovementComponent>(GetCharacterMovement());
		SetMovementModeTag(BEMoveComp->MovementMode, BEMoveComp->CustomMovementMode, true);
	}
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


void ABECharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	HealthComponent->DamageSelfDestruct(/*bFellOutOfWorld=*/ true);
}

void ABECharacter::OnDeathStarted(AActor*)
{
	DisableMovementAndCollision();
}

void ABECharacter::OnDeathFinished(AActor*)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
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

void ABECharacter::DestroyDueToDeath()
{
	K2_OnDeathFinished();

	UninitAndDestroy();
}

void ABECharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		if (BEASC->GetAvatarActor() == this)
		{
			PawnExtComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}

void ABECharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	UBECharacterMovementComponent* BEMoveComp = CastChecked<UBECharacterMovementComponent>(GetCharacterMovement());

	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);
	SetMovementModeTag(BEMoveComp->MovementMode, BEMoveComp->CustomMovementMode, true);
}

void ABECharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		const FBEGameplayTags& GameplayTags = FBEGameplayTags::Get();
		const FGameplayTag* MovementModeTag = nullptr;

		if (MovementMode == MOVE_Custom)
		{
			MovementModeTag = GameplayTags.CustomMovementModeTagMap.Find(CustomMovementMode);
		}
		else
		{
			MovementModeTag = GameplayTags.MovementModeTagMap.Find(MovementMode);
		}

		if (MovementModeTag && MovementModeTag->IsValid())
		{
			BEASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
		}
	}
}

void ABECharacter::OnRep_ReplicatedAcceleration()
{
	if (UBECharacterMovementComponent* BEMovementComponent = Cast<UBECharacterMovementComponent>(GetCharacterMovement()))
	{
		// Decompress Acceleration
		const double MaxAccel = BEMovementComponent->MaxAcceleration;
		const double AccelXYMagnitude = double(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; // [0, 255] -> [0, MaxAccel]
		const double AccelXYRadians = double(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;     // [0, 255] -> [0, 2PI]

		FVector UnpackedAcceleration(FVector::ZeroVector);
		FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
		UnpackedAcceleration.Z = double(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; // [-127, 127] -> [-MaxAccel, MaxAccel]

		BEMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
	}
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


void ABECharacter::OnRep_IsRunning()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			if (bIsRunning)
			{
				BECMC->bWantsToRun = true;
				BECMC->Run(true);
			}
			else
			{
				BECMC->bWantsToRun = false;
				BECMC->UnRun(true);
			}
		}
	}
}

void ABECharacter::Run()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			BECMC->bWantsToRun = true;
		}
	}
}

void ABECharacter::UnRun()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			BECMC->bWantsToRun = false;
		}
	}
}

void ABECharacter::OnStartRun()
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Running, 1);
	}

	OnRunChanged.Broadcast(true);

	K2_OnStartRun();
}

void ABECharacter::OnEndRun()
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Running, 0);
	}

	OnRunChanged.Broadcast(false);

	K2_OnEndRun();
}


void ABECharacter::OnRep_IsAiming()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			if (bIsAiming)
			{
				BECMC->bWantsToAim = true;
				BECMC->Aim(true);
			}
			else
			{
				BECMC->bWantsToAim = false;
				BECMC->UnAim(true);
			}
		}
	}
}

void ABECharacter::Aim()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			BECMC->bWantsToAim = true;
		}
	}
}

void ABECharacter::UnAim()
{
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		if (UBECharacterMovementComponent* BECMC = Cast<UBECharacterMovementComponent>(CMC))
		{
			BECMC->bWantsToAim = false;
		}
	}
}

void ABECharacter::OnStartAim()
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Aiming, 1);
	}

	OnAimChanged.Broadcast(true);

	K2_OnStartAim();
}

void ABECharacter::OnEndAim()
{
	if (UBEAbilitySystemComponent* BEASC = GetBEAbilitySystemComponent())
	{
		BEASC->SetLooseGameplayTagCount(TAG_Status_Aiming, 0);
	}

	OnAimChanged.Broadcast(false);

	K2_OnEndAim();
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
	return Cast<UBEAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* ABECharacter::GetAbilitySystemComponent() const
{
	return PawnExtComponent->GetBEAbilitySystemComponent();
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
