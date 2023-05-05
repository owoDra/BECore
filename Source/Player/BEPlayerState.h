// Copyright Eigi Chin

#pragma once

#include "ModularPlayerState.h"

#include "Message/BEVerbMessage.h"
#include "Team/BETeamAgentInterface.h"
#include "Ability/BEAbilitySet.h"

#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "HAL/Platform.h"
#include "System/GameplayTagStack.h"
#include "Templates/Casts.h"
#include "UObject/UObjectGlobals.h"

#include "BEPlayerState.generated.h"

class UBEPawnData;
class ABEPlayerController;
class UBEExperienceDefinition;
class UBEAbilitySystemComponent;
class UAbilitySystemComponent;
class AController;
class APlayerState;
class FName;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FBEAbilitySet_GrantedHandles;


/**
 * EBEPlayerConnectionType
 *
 *	Client ���Q�[���ɂǂ̖����Őڑ�(�Q��)���Ă��邩������
 */
UENUM()
enum class EBEPlayerConnectionType : uint8
{
	// ���ۂɃQ�[���ɎQ�����Ċ������Ă���v���C���[
	Player = 0,

	// ���A���^�C���ȃQ�[���ɐڑ����Ă��邪�Q���͂��Ă��Ȃ��ϐ��
	LiveSpectator,

	// �L�^�����Q�[�������v���C���Ă���ϐ��
	ReplaySpectator,

	// ���ɃQ�[������ؒf����Ă���v���C���[
	InactivePlayer
};


/**
 * ABEPlayerState
 *
 *	���̃v���W�F�N�g�Ŏg�p�����x�[�X�� PlayerState �N���X�B
 */
UCLASS(Config = Game)
class BECORE_API ABEPlayerState 
	: public AModularPlayerState
	, public IAbilitySystemInterface
	, public IBETeamAgentInterface
{
	GENERATED_BODY()

public:
	ABEPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const FName NAME_BEAbilityReady;

public:
	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor interface

	//~APlayerState interface
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnDeactivated() override;
	virtual void OnReactivated() override;
	//~End of APlayerState interface

private:
	void OnExperienceLoaded(const UBEExperienceDefinition* CurrentExperience);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBEAbilitySystemComponent> AbilitySystemComponent;

	FBEAbilitySet_GrantedHandles PawnDataAbilityHandles;


private:
	// Player �� Pawn �܂��� Character �̃X�|�[���ɗp���� ��{���B
	// GameMode �͂��� PawnData �̏������Ƃ� Player �� Pawn �܂��� Character ���X�|�[��������
	UPROPERTY(Replicated)
	TObjectPtr<const UBEPawnData> PawnData;

public:
	/**
	 * SetPawnData
	 *
	 *	Player �� Pawn �܂��� Character �̃X�|�[���ɗp����PawnData ��ݒ肷��B
	 *  ���s�ɂ̓T�[�o�[�������K�v�B
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "PlayerState", meta = (DisplayName = "SetPawnData"))
	void K2_SetPawnData(const UBEPawnData* InPawnData);
	void SetPawnData(const UBEPawnData* InPawnData);

	/**
	 * GetPawnData
	 *
	 *	Player �� Pawn �܂��� Character �̃X�|�[���ɗp����PawnData ��Ԃ�
	 */
	const UBEPawnData* GetPawnData() const { return PawnData; }

	
private:
	// Player �̃Q�[���ւ̐ڑ�(�Q��)���
	UPROPERTY(Replicated)
	EBEPlayerConnectionType MyPlayerConnectionType;

public:
	/**
	 * SetPlayerConnectionType
	 *
	 *	Player �̃Q�[���ւ̐ڑ�(�Q��)��Ԃ�ݒ肷��
	 */
	void SetPlayerConnectionType(EBEPlayerConnectionType NewType);

	/**
	 * GetPlayerConnectionType
	 *
	 *	Player �̃Q�[���ւ̐ڑ�(�Q��)��Ԃ�Ԃ�
	 */
	EBEPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }
	

private:
	// Player �������� Team ���ύX���ꂽ���ɒm�点�邽�߂̃f���Q�[�g
	UPROPERTY()
	FOnBETeamIndexChangedDelegate OnTeamChangedDelegate;

	// PLayer �������� Team �� ID
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	// Player �������� Team ���ł� �O���[�v�����p�� ID
	// ��{�I�ɂ͑�l���ł̑ΐ�Q�[���ȂǂŎg�p����B
	UPROPERTY(ReplicatedUsing = OnRep_MySquadID)
	int32 MySquadID;

	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);

	UFUNCTION()
	void OnRep_MySquadID();

public:
	//~IBETeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnBETeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IBETeamAgentInterface interface

	/**
	 * GetTeamId
	 *
	 *	Player �̑����� Team �� ID �𐮐��l�Ƃ��ĕԂ�
	 */
	UFUNCTION(BlueprintCallable)
	int32 GetTeamId() const { return GenericTeamIdToInteger(MyTeamID); }

	/**
	 * SetSquadID
	 *
	 *	Player �������� Team ���ł� �O���[�v�����p�� ID ��ݒ肷��
	 */
	void SetSquadID(int32 NewSquadID);

	/**
	 * SetSquadID
	 *
	 *	Player �������� Team ���ł� �O���[�v�����p�� ID ��Ԃ�
	 */
	UFUNCTION(BlueprintCallable)
	int32 GetSquadId() const { return MySquadID; }


private:
	// Player ���ۗL���� ���v���Ƃ��ăJ�E���g�\�� Tag �̏��
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

protected:
	const FGameplayTagStackContainer& GetStatTags() const { return StatTags; }
	void SetStatTags(const FGameplayTagStackContainer& InStatTags) { StatTags = InStatTags; }

public:
	/**
	 * AddStatTagStack
	 *
	 * Player �ɓ��v���Ƃ��Ĉ����� Tag ��ǉ����� (StackCount �� 0 �ȉ��̏ꍇ�͉������Ȃ�)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	/**
	 * RemoveStatTagStack
	 *
	 * Player �ɓ��v���Ƃ��Ĉ����� Tag ���폜���� (StackCount �� 0 �ȉ��̏ꍇ�͉������Ȃ�)
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	/**
	 * GetStatTagStackCount
	 *
	 * Player �ɓ��v���Ƃ��Ĉ����� Tag ���������邩��Ԃ� (���݂��Ȃ��ꍇ�� 0 ��Ԃ�)
	 */
	UFUNCTION(BlueprintCallable, Category=Teams)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	/**
	 * HasStatTag
	 *
	 * Player �ɓ��v���Ƃ��Ĉ����� Tag �����݂��邩�ǂ�����Ԃ�
	 */
	UFUNCTION(BlueprintCallable, Category=Teams)
	bool HasStatTag(FGameplayTag Tag) const;

	
public:
	/**
	 * ClientBroadcastMessage
	 *
	 * ���� Player �ɂ̂݃��b�Z�[�W�𑗐M����B
	 * ���̃v���C���[����̏̎^�ȂǂƂ������A�K�������d�v�ł͂Ȃ������ɂ̂ݎg�p
	 */
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "PlayerState")
	void ClientBroadcastMessage(const FBEVerbMessage Message);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	ABEPlayerController* GetBEPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	UBEAbilitySystemComponent* GetBEAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};
