// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

//LYRAGAME_API起到导出宏的作用
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraExperience, Log, All);
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraAbilitySystem, Log, All);
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraTeams, Log, All);

// 传递一个上下文对象,来判断当前的世界是客户端还是服务端
// 该函数在LyraExperienceManagerComponent.h中调用,用来打印Experience的加载日志.传递的对象是GameState.该对象具有网络同步的属性.
FString GetClientServerContextString(UObject* ContextObject = nullptr);
