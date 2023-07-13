// Copyright owoDra

#pragma once

#include "CommonListView.h"
#include "Components/ListView.h"

#include "BEListView.generated.h"

class UUserWidget;
class ULocalPlayer;
class UBEWidgetFactory;

UCLASS(meta = (DisableNativeTick))
class BECORE_API UBEListView : public UCommonListView
{
	GENERATED_BODY()

public:
	UBEListView(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const override;
#endif

protected:
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;
	//virtual bool OnIsSelectableOrNavigableInternal(UObject* SelectedItem) override;

protected:
	UPROPERTY(EditAnywhere, Instanced, Category="Entry Creation")
	TArray<TObjectPtr<UBEWidgetFactory>> FactoryRules;
};