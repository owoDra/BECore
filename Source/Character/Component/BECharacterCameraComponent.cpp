// Copyright Eigi Chin

#include "BECharacterCameraComponent.h"

#include "Character/Component/BECharacterBasicComponent.h"
#include "Character/BECharacterData.h"
#include "Camera/Mode/BECameraMode.h"
#include "BEGameplayTags.h"

#include "Camera/CameraTypes.h"
#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Misc/AssertionMacros.h"
#include "Templates/Casts.h"
#include "UObject/UObjectBaseUtility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BECharacterCameraComponent)

const FName UBECharacterCameraComponent::NAME_ActorFeatureName("CharacterCamera");


UBECharacterCameraComponent::UBECharacterCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UBECharacterCameraComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetOwner<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("UBECharacterCameraComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> Components;
	Pawn->GetComponents(UBECharacterCameraComponent::StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one UBECharacterCameraComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// コンポーネントが GameWorld に存在する際に　InitStateSystem に登録する
	RegisterInitStateFeature();
}

void UBECharacterCameraComponent::BeginPlay()
{
	// すべての Feature への変更をリッスンする
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// InisStateSystem にこのコンポーネントがスポーンしたことを知らせる。
	ensure(TryToChangeInitState(TAG_InitState_Spawned));

	// 残りの初期化を行う
	CheckDefaultInitialization();
}

void UBECharacterCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


bool UBECharacterCameraComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetOwner<APawn>();

	/**
	 * [InitState None] -> [InitState Spawned]
	 *
	 *  Pawn が有効
	 */
	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		if (Pawn != nullptr)
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
		// シミュレーションでなく、かつ Controller が有効
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = Pawn->GetController();
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
		return Manager->HasFeatureReachedInitState(Pawn, UBECharacterBasicComponent::NAME_ActorFeatureName, TAG_InitState_DataInitialized);
	}

	/**
	 * [InitState DataInitialized] -> [InitState GameplayReady]
	 *
	 *  無条件に許可
	 */
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UBECharacterCameraComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// 特に何もしない
}

void UBECharacterCameraComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// CharacterBasicComponent が DataInitialized に到達しているか
	// つまり、他のすべての Feature が DataAvailable に到達しているか
	if (Params.FeatureName == UBECharacterBasicComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataInitialized)
		{
			CheckDefaultInitialization();
		}
	}
}

void UBECharacterCameraComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}


void UBECharacterCameraComponent::SetAbilityCameraMode(TSubclassOf<UBECameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		AbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void UBECharacterCameraComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		AbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}

void UBECharacterCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	check(CameraModeStack);

	if (const TSubclassOf<UBECameraMode> CameraMode = DetermineCameraMode())
	{
		PushCameraMode(CameraMode);
	}

	ComputeCameraView(DeltaTime, DesiredView);
}

TSubclassOf<UBECameraMode> UBECharacterCameraComponent::DetermineCameraMode() const
{
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	const APawn* Pawn = GetOwner<APawn>();
	if (!Pawn)
	{
		return nullptr;
	}

	if (UBECharacterBasicComponent* CharacterBasic = UBECharacterBasicComponent::FindCharacterBasicComponent(Pawn))
	{
		if (const UBECharacterData* CharacterData = CharacterBasic->GetCharacterData<UBECharacterData>())
		{
			return CharacterData->DefaultCameraMode;
		}
	}

	return nullptr;
}
