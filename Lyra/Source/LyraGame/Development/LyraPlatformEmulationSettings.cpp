// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPlatformEmulationSettings.h"
#include "CommonUIVisibilitySubsystem.h"
#include "Engine/PlatformSettingsManager.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "DeviceProfiles/DeviceProfile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPlatformEmulationSettings)

#define LOCTEXT_NAMESPACE "LyraCheats"


ULyraPlatformEmulationSettings::ULyraPlatformEmulationSettings()
{
}

FName ULyraPlatformEmulationSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

FName ULyraPlatformEmulationSettings::GetPretendBaseDeviceProfile() const
{
	return PretendBaseDeviceProfile;
}

FName ULyraPlatformEmulationSettings::GetPretendPlatformName() const
{
	return PretendPlatform;
}


#if WITH_EDITOR

void ULyraPlatformEmulationSettings::OnPlayInEditorStarted() const
{
	// Show a notification toast to remind the user that there's a tag enable override set
	// 显示一条通知提示信息，提醒用户已设置标签启用优先级调整功能
	if (!AdditionalPlatformTraitsToEnable.IsEmpty())
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("PlatformTraitEnableActive", "Platform Trait Override\nEnabling {0}"),
			FText::AsCultureInvariant(AdditionalPlatformTraitsToEnable.ToStringSimple())
		));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	// Show a notification toast to remind the user that there's a tag suppression override set
	// 显示一条通知提示信息，提醒用户已设置标签抑制覆盖功能
	if (!AdditionalPlatformTraitsToSuppress.IsEmpty())
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("PlatformTraitSuppressionActive", "Platform Trait Override\nSuppressing {0}"),
			FText::AsCultureInvariant(AdditionalPlatformTraitsToSuppress.ToStringSimple())
		));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	// Show a notification toast to remind the user that there's a platform override set
	// 显示一条通知提示信息，提醒用户已设置平台覆盖选项
	if (PretendPlatform != NAME_None)
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("PlatformOverrideActive", "Platform Override Active\nPretending to be {0}"),
			FText::FromName(PretendPlatform)
		));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	
}

void ULyraPlatformEmulationSettings::ApplySettings()
{
	// 是遏制CommonUI对象调试Tag
	UCommonUIVisibilitySubsystem::SetDebugVisibilityConditions(AdditionalPlatformTraitsToEnable, AdditionalPlatformTraitsToSuppress);

	if (GIsEditor && PretendPlatform != LastAppliedPretendPlatform)
	{
		ChangeActivePretendPlatform(PretendPlatform);
	}
	
	PickReasonableBaseDeviceProfile();
}
#endif

#if WITH_EDITOR
void ULyraPlatformEmulationSettings::ChangeActivePretendPlatform(FName NewPlatformName)
{
	LastAppliedPretendPlatform = NewPlatformName;
	PretendPlatform = NewPlatformName;
	
	// 设置编辑器模拟的平台
	UPlatformSettingsManager::SetEditorSimulatedPlatform(PretendPlatform);
}

void ULyraPlatformEmulationSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ApplySettings();
}

void ULyraPlatformEmulationSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);

	ApplySettings();
}

void ULyraPlatformEmulationSettings::PostInitProperties()
{
	Super::PostInitProperties();

	ApplySettings();
}


#endif


TArray<FName> ULyraPlatformEmulationSettings::GetKnownPlatformIds() const
{
	TArray<FName> Results;
	
#if WITH_EDITOR
	Results.Add(NAME_None);
	Results.Append(UPlatformSettingsManager::GetKnownAndEnablePlatformIniNames());
#endif

	return Results;
}

TArray<FName> ULyraPlatformEmulationSettings::GetKnownDeviceProfiles() const
{
	TArray<FName> Results;
	
#if WITH_EDITOR
	const UDeviceProfileManager& Manager = UDeviceProfileManager::Get();
	Results.Reserve(Manager.Profiles.Num() + 1);
	
	if (PretendPlatform == NAME_None)
	{
		Results.Add(NAME_None);
	}

	for (const TObjectPtr<UDeviceProfile>& Profile : Manager.Profiles)
	{
		bool bIncludeEntry = true;
		if (PretendPlatform != NAME_None)
		{
			if (Profile->DeviceType != PretendPlatform.ToString())
			{
				bIncludeEntry = false;
			}
		}

		if (bIncludeEntry)
		{
			Results.Add(Profile->GetFName());
		}
	}

	
#endif
	return Results;
}




void ULyraPlatformEmulationSettings::PickReasonableBaseDeviceProfile()
{
	// First see if our pretend device profile is already compatible, if so we don't need to do anything
	// 首先检查我们的模拟设备配置是否已经兼容，如果已经兼容，那么我们无需再进行任何操作

		
	UDeviceProfileManager& Manager = UDeviceProfileManager::Get();
	if (UDeviceProfile* ProfilePtr = Manager.FindProfile(PretendBaseDeviceProfile.ToString(), /*bCreateOnFail=*/ false))
	{
		const bool bIsCompatible = (PretendPlatform == NAME_None) || (ProfilePtr->DeviceType == PretendPlatform.ToString());
		
		if (!bIsCompatible)
		{
			PretendBaseDeviceProfile = NAME_None;
		}
		
	}

	//不兼容的情况下，查找对应的 名称最短的配置文件【一种简单的判断准则
	if ((PretendPlatform != NAME_None) && (PretendBaseDeviceProfile == NAME_None))
	{
		// If we're pretending we're a platform and don't have a pretend base profile, pick a reasonable one,
		// preferring the one with the shortest name as a simple heuristic

		// 如果我们假定自己是一个平台，并且没有虚拟的用户资料信息，那就选择一个合理的名称即可，
		// 更倾向于选择名称最短的那个，这是一种简单的判断准则。


		FName ShortestMatchingProfileName;
		const FString PretendPlatformStr = PretendPlatform.ToString();
		for (const TObjectPtr<UDeviceProfile>& Profile : Manager.Profiles)
		{
			// 平台类型匹配找最短的即可
			if (Profile->DeviceType == PretendPlatformStr)
			{
				const FName TestName = Profile->GetFName();
				
				if ((ShortestMatchingProfileName == NAME_None) || (TestName.GetStringLength() < ShortestMatchingProfileName.GetStringLength()))
				{
					ShortestMatchingProfileName = TestName;
				}
				
			}

			
		}
		PretendBaseDeviceProfile = ShortestMatchingProfileName;
	}
	
}




#undef LOCTEXT_NAMESPACE