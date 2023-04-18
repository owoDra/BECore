// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "BEDamagePopStyleNiagara.generated.h"

class UNiagaraSystem;

///////////////////////////////////////////////

/**
 * ポップ スタイルは、ダメージ システムの表現に使用するナイアガラ アセットを定義するために使用されます。
 */
UCLASS()
class UBEDamagePopStyleNiagara : public UDataAsset
{
	GENERATED_BODY()

public:

	// ダメージ情報を格納するナイアガラ配列パラメーターの名前
	UPROPERTY(EditDefaultsOnly, Category="DamagePop")
	FName DamageParamName = FName(TEXT("Damage"));

	// ダメージをリセットするまでの時間
	UPROPERTY(EditDefaultsOnly, Category = "DamagePop")
	float DamageResetInteval = 1.2f;

	// ダメージを表示する NiagaraSystem
	UPROPERTY(EditDefaultsOnly, Category="DamagePop")
	TObjectPtr<UNiagaraSystem> TextNiagara;
};
