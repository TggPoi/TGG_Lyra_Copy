// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSettingsLocal.h"

ULyraSettingsLocal::ULyraSettingsLocal()
{
	
}

ULyraSettingsLocal* ULyraSettingsLocal::Get()
{
	return GEngine ? CastChecked<ULyraSettingsLocal>(GEngine->GetGameUserSettings()) : nullptr;
}