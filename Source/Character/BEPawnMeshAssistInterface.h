// Copyright Eigi Chin

#pragma once

#include "UObject/Interface.h"

#include "Containers/Array.h"

#include "BEPawnMeshAssistInterface.generated.h"

class USkeletalMeshComponent;
class UBEAnimInstance;


/**
 * UBECameraAssistInterface / IBECameraAssistInterface
 * 
 *  Pawn �܂��� Character �� SkeltalMesh �Ɋւ���⏕���s���C���^�[�t�F�[�X
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UBEPawnMeshAssistInterface : public UInterface
{
	GENERATED_BODY()
};

class IBEPawnMeshAssistInterface
{
	GENERATED_BODY()
public:
	/**
	 * GetMeshes
	 * 
	 *  Pawn �܂��� Character �̌����ڂ�\�� Mesh �����ׂĎ擾����
	 *  ��ʓI�ɂ���ɂ� TPP Mesh �� FPP Mesh���܂܂�܂��B
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	void GetMeshes(TArray<USkeletalMeshComponent*>& Meshes) const;

	/**
	 * GetFPPMesh
	 *
	 *  Pawn �܂��� Character �� FPP���_�ɂ����錩���ڂ�\�� Mesh ��Ԃ��B
	 *  FPP ���_�ł� Mesh ���g�p���Ȃ��ꍇ�� nullptr ��Ԃ��B
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	USkeletalMeshComponent* GetFPPMesh() const;

	/**
	 * GetTPPMesh
	 *
	 *  Pawn �܂��� Character �� TPP���_�ɂ����錩���ڂ�\�� Mesh ��Ԃ��B
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	USkeletalMeshComponent* GetTPPMesh() const;


	/**
	 * GetMainAnimInstances
	 *
	 *  Pawn �܂��� Character �� CopyPose �Ȃǂł͂Ȃ����C���� AnimInstance ���擾����B
	 *  ��ʓI�� FPP Mesh ����� TPP Mesh �� AnimInstance ���܂܂��B
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	void GetMainAnimInstances(TArray<UBEAnimInstance*>& Instances) const;

	/**
	 * GetFPPAnimInstance
	 *
	 *  Pawn �܂��� Character �� FPP���_�ɂ����� Mesh �� AnimInstance ��Ԃ��B
	 *  FPP ���_�ł� Mesh ���g�p���Ȃ��ꍇ�� nullptr ��Ԃ��B
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	UBEAnimInstance* GetFPPAnimInstance() const;

	/**
	 * GetTPPAnimInstance
	 *
	 *  Pawn �܂��� Character �� TPP���_�ɂ����� Mesh �� AnimInstance ��Ԃ��B
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	UBEAnimInstance* GetTPPAnimInstance() const;
};
