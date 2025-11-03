// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraTeamAgentInterface.h"

#include "LyraLogChannels.h"
#include "UObject/ScriptInterface.h"

ULyraTeamAgentInterface::ULyraTeamAgentInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void ILyraTeamAgentInterface::ConditionalBroadcastTeamChanged(TScriptInterface<ILyraTeamAgentInterface> This,
	FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
	//判断队伍是否真的发生变化
	if (OldTeamID != NewTeamID)
	{
		const int32 OldTeamIndex = GenericTeamIdToInteger(OldTeamID); 
		const int32 NewTeamIndex = GenericTeamIdToInteger(NewTeamID);

		UObject* ThisObj = This.GetObject();
		UE_LOG(LogLyraTeams, Verbose, TEXT("[%s] %s assigned team %d"), *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamIndex);

		This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);
		
	}
	
}
