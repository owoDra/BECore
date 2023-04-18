// Copyright Eigi Chin

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "BENumberPopComponent.h"
#include "BENumberPopComponent_NiagaraText.generated.h"

class UBEDamagePopStyleNiagara;

////////////////////////////////////////

UCLASS(Blueprintable)
class UBENumberPopComponent_NiagaraText : public UBENumberPopComponent
{
	GENERATED_BODY()

public:

	UBENumberPopComponent_NiagaraText(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UBENumberPopComponent interface
	virtual void AddNumberPop(const FBENumberPopRequest& NewRequest) override;
	//~End of UBENumberPopComponent interface

protected:
	void HandleDamageReset();

protected:
	/** Style patterns to attempt to apply to the incoming number pops */
	UPROPERTY(EditDefaultsOnly, Category = "Number Pop|Style")
	UBEDamagePopStyleNiagara* Style;

	//Niagara Component used to display the damage
	UPROPERTY(EditDefaultsOnly, Category = "Number Pop|Style")
	UNiagaraComponent* NiagaraComp;

private:
	FTimerHandle DamageResetTimerHandle;

	float TotalDamage = 0.0f;
};
