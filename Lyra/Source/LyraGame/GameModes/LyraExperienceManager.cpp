// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraExperienceManager.h"
#include "Engine/Engine.h"
#include "Subsystems/SubsystemCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraExperienceManager)

#if WITH_EDITOR

void ULyraExperienceManager::OnPlayInEditorBegun()
{
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void ULyraExperienceManager::NotifyOfPluginActivation(const FString PluginURL)
{
	if (GIsEditor)
	{
		ULyraExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<ULyraExperienceManager>();
		check(ExperienceManagerSubsystem);

		// Track the number of requesters who activate this plugin. Multiple load/activation requests are always allowed because concurrent requests are handled.
		// 记录激活此插件的请求者的数量。由于可以处理并发请求，因此允许多次加载/激活操作。
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);
		
		++Count;
	}
	
}

bool ULyraExperienceManager::RequestToDeactivatePlugin(const FString PluginURL)
{
	if (GIsEditor)
	{
		ULyraExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<ULyraExperienceManager>();
		check(ExperienceManagerSubsystem);
		
		// Only let the last requester to get this far deactivate the plugin
		// 只允许最后提出请求的用户能够继续进行到这一步，并且由其来解除该插件的激活状态。
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
		--Count;
		
		if (Count == 0)
		{
			ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.Remove(PluginURL);
			return true;
		}
		
		return false;
		
	}
	return true;
}
#endif
