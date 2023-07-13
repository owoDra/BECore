// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright owoDra

#pragma once

#include "GenericTeamAgentInterface.h"

#include "CoreTypes.h"
#include "Delegates/Delegate.h"
#include "Misc/AssertionMacros.h"
#include "UObject/Object.h"
#include "UObject/ScriptInterface.h"
#include "UObject/UObjectGlobals.h"

#include "BETeamAgentInterface.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBETeamIndexChangedDelegate, UObject*, ObjectChangingTeam, int32, OldTeamID, int32, NewTeamID);


inline int32 GenericTeamIdToInteger(FGenericTeamId ID)
{
	return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : (int32)ID;
}

inline FGenericTeamId IntegerToGenericTeamId(int32 ID)
{
	return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId((uint8)ID);
}

/** Interface for actors which can be associated with teams */
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UBETeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_UINTERFACE_BODY()
};

class BECORE_API IBETeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_IINTERFACE_BODY()

	virtual FOnBETeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

	static void ConditionalBroadcastTeamChanged(TScriptInterface<IBETeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);
	
	FOnBETeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
	{
		FOnBETeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
		check(Result);
		return *Result;
	}
};
