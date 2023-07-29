// Copyright owoDra

#pragma once

#include "UObject/Interface.h"

#include "Containers/Array.h"

#include "BEPawnMeshAssistInterface.generated.h"

class USkeletalMeshComponent;
class UBEAnimInstance;


/**
 * UBECameraAssistInterface / IBECameraAssistInterface
 * 
 *  Pawn または Character の SkeltalMesh に関する補助を行うインターフェース
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
	 *  Pawn または Character の見た目を表す Mesh をすべて取得する
	 *  一般的にこれには TPP Meshes と FPPes Meshが含まれます
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	void GetMeshes(TArray<USkeletalMeshComponent*>& Meshes) const;

	/**
	 * GetFPPMeshes
	 *
	 *  Pawn または Character の FPP視点における見た目を表す Mesh をすべて返す
	 *  FPP 視点での Mesh を使用しない場合は空の配列を返す
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	void GetFPPMeshes(TArray<USkeletalMeshComponent*>& Meshes) const;

	/**
	 * GetTPPMesh
	 *
	 *  Pawn または Character の TPP視点における見た目を表す Mesh をすべて返す
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	void GetTPPMeshes(TArray<USkeletalMeshComponent*>& Meshes) const;

	/**
	 * GetFPPFirstMesh
	 *
	 *  Pawn または Character の FPP視点における見た目を表す Mesh を返す
	 *  FPP 視点での Mesh を使用しない場合は nullptr を返す
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	void GetFPPFirstMesh(USkeletalMeshComponent*& Mesh) const;

	/**
	 * GetTPPFirstMesh
	 *
	 *  Pawn または Character の TPP視点における見た目を表す Mesh を返す
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	void GetTPPFirstMesh(USkeletalMeshComponent*& Mesh) const;
	

public:
	/**
	 * GetMainAnimInstance
	 *
	 *  Pawn または Character の CopyPose などではないメインの AnimInstance を取得する
	 *  一般的に TPP Mesh の AnimInstance が含まれる
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	void GetMainAnimInstance(UBEAnimInstance*& Instance) const;

	/**
	 * GetSubAnimInstances
	 *
	 *  Pawn または Character の CopyPose などサブの AnimInstance を全て取得する
	 *  一般的に FPP Mesh の AnimInstance が含まれる
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	void GetSubAnimInstances(TArray<UAnimInstance*>& Instances) const;

	/**
	 * GetTPPAnimInstance
	 *
	 *  Pawn または Character の TPP Mesh の AnimInstance を取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	void GetTPPAnimInstance(UAnimInstance*& Instance) const;


	/**
	 * GetFPPAnimInstance
	 *
	 *  Pawn または Character の FPP Mesh の AnimInstance を取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	void GetFPPAnimInstance(UAnimInstance*& Instance) const;
};
