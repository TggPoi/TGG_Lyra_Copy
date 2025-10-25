// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
// 001 Not Finished.只写了基础类
// 002 基本完成了.但是有些变量和函数,目前用不到.后面会单独抽几章去讲.比如GAS相关和回放相关的.
#pragma once

#include "AbilitySystemInterface.h"
#include "ModularGameState.h"

#include "LyraGameState.generated.h"

#define UE_API LYRAGAME_API

struct FLyraVerbMessage;

class APlayerState;
class UAbilitySystemComponent;
class ULyraAbilitySystemComponent;
class ULyraExperienceManagerComponent;
class UObject;
struct FFrame;

/**
 * ALyraGameState
 *
 *	The base game state class used by this project.
 * 本项目所使用的基础游戏状态类。
 */
UCLASS(MinimalAPI, Config = Game)
class ALyraGameState : public AModularGameStateBase
{
	GENERATED_BODY()
	
public:
	/*
	 * 构造函数
	 * 1.开启Tick的功能
	 * 2.初始化GAS组件 设置GAS的同步模式
	 * 3.创建体验管理组件 负载加载Experience
	 */
	UE_API ALyraGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

		
};



#undef UE_API