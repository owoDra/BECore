// Copyright Eigi Chin

#pragma once

#include "Camera/BECameraComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"

#include "BEPawnCameraComponent.generated.h"

class APawn;


/**
 * UBEPawnCameraComponent
 *
 *	Character をプレイヤーが視点操作可能にするための処理を行う
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class BECORE_API UBEPawnCameraComponent : public UBECameraComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UBEPawnCameraComponent(const FObjectInitializer& ObjectInitializer);

	// このコンポーネントを実装する際の FeatureName
	static const FName NAME_ActorFeatureName;

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface


public:
	void SetAbilityCameraMode(TSubclassOf<UBECameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

protected:
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	TSubclassOf<UBECameraMode> DetermineCameraMode() const;

protected:
	// Ability によって設定される一時的な CameraMode
	TSubclassOf<UBECameraMode> AbilityCameraMode;

	// 最後に CameraMode を設定した Ability のハンドル
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;


public:
	UFUNCTION(BlueprintPure, Category = "Character")
	static UBEPawnCameraComponent* FindPawnCameraComponent(const APawn* Pawn);
};
