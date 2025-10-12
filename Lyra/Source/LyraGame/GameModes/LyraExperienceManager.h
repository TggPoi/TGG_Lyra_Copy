// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Finished.
#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "LyraExperienceManager.generated.h"


/**
 * Manager for experiences - primarily for arbitration between multiple PIE sessions
 * 体验管理的引擎子系统 - 主要负责多个 PIE 会议之间的协调工作
 */
UCLASS(MinimalAPI)
class ULyraExperienceManager : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:

#if WITH_EDITOR	
	//在编辑器模块中的StartupModule()进行调用,用于初始化GameFeaturePluginRequestCountMap
	LYRAGAME_API void OnPlayInEditorBegun();
	
	//通知有插件被激活了,增加计数,由LyraExperienceManagerComponent调用
	static void NotifyOfPluginActivation(const FString PluginURL);

	//要求取消插件的激活计数,减少计数,由LyraExperienceManagerComponent调用
	static bool RequestToDeactivatePlugin(const FString PluginURL);
	
	//运行时不需要这个功能
#else
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	static bool RequestToDeactivatePlugin(const FString PluginURL) { return true; }
#endif
	
private:
	// The map of requests to active count for a given game feature plugin
	// (to allow first in, last out activation management during PIE)
	// 指定游戏功能插件的请求量与激活次数的关系图
	// （以便在 PIE 过程中实现[先入后出]的激活管理）
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};
