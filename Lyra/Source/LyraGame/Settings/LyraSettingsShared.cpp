// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSettingsShared.h"

ULyraSettingsShared::ULyraSettingsShared()
{

}


ULyraSettingsShared* ULyraSettingsShared::CreateTemporarySettings(const ULyraLocalPlayer* LocalPlayer)
{
	return nullptr; 
}

ULyraSettingsShared* ULyraSettingsShared::LoadOrCreateSettings(const ULyraLocalPlayer* LocalPlayer)
{
	ULyraSettingsShared* Settings = nullptr;
	return Settings;
}

bool ULyraSettingsShared::AsyncLoadOrCreateSettings(const ULyraLocalPlayer* LocalPlayer,
	FOnSettingsLoadedEvent Delegate)
{
	return false;

}


