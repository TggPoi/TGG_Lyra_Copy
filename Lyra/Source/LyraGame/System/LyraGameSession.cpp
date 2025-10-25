// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameSession.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameSession)

ALyraGameSession::ALyraGameSession(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

bool ALyraGameSession::ProcessAutoLogin()
{
	// This is actually handled in LyraGameMode::TryDedicatedServerLogin
	// 实际上，这是在 LyraGameMode::TryDedicatedServerLogin 中进行处理的。
	return true;
}

void ALyraGameSession::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void ALyraGameSession::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
}
