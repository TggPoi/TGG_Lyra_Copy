// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSettingsLocal.h"

//下面这三个对应LyraPlatformEmulationSettings中的三个变量
#if WITH_EDITOR
static TAutoConsoleVariable<bool> CVarApplyFrameRateSettingsInPIE(TEXT("Lyra.Settings.ApplyFrameRateSettingsInPIE"),
																  false,
																  TEXT("Should we apply frame rate settings in PIE?"),
																  ECVF_Default);

static TAutoConsoleVariable<bool> CVarApplyFrontEndPerformanceOptionsInPIE(
	TEXT("Lyra.Settings.ApplyFrontEndPerformanceOptionsInPIE"),
	false,
	TEXT("Do we apply front-end specific performance options in PIE?"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarApplyDeviceProfilesInPIE(TEXT("Lyra.Settings.ApplyDeviceProfilesInPIE"),
															   false,
															   TEXT(
																   "Should we apply experience/platform emulated device profiles in PIE?"),
															   ECVF_Default);
#endif

ULyraSettingsLocal::ULyraSettingsLocal()
{
	
}

ULyraSettingsLocal* ULyraSettingsLocal::Get()
{
	return GEngine ? CastChecked<ULyraSettingsLocal>(GEngine->GetGameUserSettings()) : nullptr;
}