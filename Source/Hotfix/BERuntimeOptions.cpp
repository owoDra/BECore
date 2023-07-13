// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#include "BERuntimeOptions.h"

#include "Containers/UnrealString.h"
#include "HAL/Platform.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BERuntimeOptions)


UBERuntimeOptions::UBERuntimeOptions()
{
	OptionCommandPrefix = TEXT("ro");
}

UBERuntimeOptions* UBERuntimeOptions::GetRuntimeOptions()
{
	return GetMutableDefault<UBERuntimeOptions>();
}

const UBERuntimeOptions& UBERuntimeOptions::Get()
{
	const UBERuntimeOptions& RuntimeOptions = *GetDefault<UBERuntimeOptions>();
	return RuntimeOptions;
}