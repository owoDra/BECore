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
	 *  一般的にこれには TPP Mesh と FPP Meshが含まれます。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	void GetMeshes(TArray<USkeletalMeshComponent*>& Meshes) const;

	/**
	 * GetFPPMesh
	 *
	 *  Pawn または Character の FPP視点における見た目を表す Mesh を返す。
	 *  FPP 視点での Mesh を使用しない場合は nullptr を返す。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	USkeletalMeshComponent* GetFPPMesh() const;

	/**
	 * GetTPPMesh
	 *
	 *  Pawn または Character の TPP視点における見た目を表す Mesh を返す。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Mesh")
	USkeletalMeshComponent* GetTPPMesh() const;


	/**
	 * GetMainAnimInstances
	 *
	 *  Pawn または Character の CopyPose などではないメインの AnimInstance を取得する。
	 *  一般的に FPP Mesh および TPP Mesh の AnimInstance が含まれる。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	void GetMainAnimInstances(TArray<UBEAnimInstance*>& Instances) const;

	/**
	 * GetFPPAnimInstance
	 *
	 *  Pawn または Character の FPP視点における Mesh の AnimInstance を返す。
	 *  FPP 視点での Mesh を使用しない場合は nullptr を返す。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	UBEAnimInstance* GetFPPAnimInstance() const;

	/**
	 * GetTPPAnimInstance
	 *
	 *  Pawn または Character の TPP視点における Mesh の AnimInstance を返す。
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pawn|Anim")
	UBEAnimInstance* GetTPPAnimInstance() const;
};
