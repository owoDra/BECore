// Copyright owoDra

#include "BENumberPopComponent_NiagaraText.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "BEDamagePopStyleNiagara.h"
#include "BELogChannels.h"

////////////////////////////////////////

UBENumberPopComponent_NiagaraText::UBENumberPopComponent_NiagaraText(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBENumberPopComponent_NiagaraText::AddNumberPop(const FBENumberPopRequest& NewRequest)
{
	if (Style)
	{
		TotalDamage = FMath::Abs(TotalDamage) + NewRequest.NumberToDisplay;

		// クリティカルダメージの場合に負の値として NiagaraSystem で判定できるようにする。
		TotalDamage = (NewRequest.bIsCriticalDamage) ? (-TotalDamage) : (TotalDamage);
		
		// NiagaraComponent が無い場合に新しく作成
		if (!NiagaraComp)
		{
			NiagaraComp = NewObject<UNiagaraComponent>(GetOwner());
			NiagaraComp->SetAsset(Style->TextNiagara);
			NiagaraComp->bAutoActivate = false;
			NiagaraComp->SetupAttachment(nullptr);
			check(NiagaraComp);
			NiagaraComp->RegisterComponent();
		}

		NiagaraComp->SetWorldLocation(NewRequest.WorldLocation);

		TArray<float> DamageInfo;
		DamageInfo.EmplaceAt(0, TotalDamage);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComp, Style->DamageParamName, DamageInfo);

		if (!NiagaraComp->IsActive())
		{
			NiagaraComp->Activate(false);
		}

		if (UWorld* World = GetWorld())
		{
			if (DamageResetTimerHandle.IsValid())
			{
				World->GetTimerManager().ClearTimer(DamageResetTimerHandle);
			}

			World->GetTimerManager().SetTimer(DamageResetTimerHandle, this, &ThisClass::HandleDamageReset, Style->DamageResetInteval);
		}
	}
}

void UBENumberPopComponent_NiagaraText::HandleDamageReset()
{
	if (Style)
	{
		TotalDamage = 0.0f;
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComp, Style->DamageParamName, TArray<float>());
	}
}
