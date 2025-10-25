// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Not Finished.只写了基础类
// 002 还需要等待Charccter初始化过程,以及创建ASC的属性集的定义
#pragma once

#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "System/GameplayTagStack.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraPlayerState.generated.h"

#define UE_API LYRAGAME_API

struct FLyraVerbMessage;

class AController;
class ALyraPlayerController;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class ULyraAbilitySystemComponent;
class ULyraExperienceDefinition;
class ULyraPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;

/** Defines the types of client connected */
/** 定义已连接客户端的类型 */
UENUM()
enum class ELyraPlayerConnectionType : uint8
{
	// An active player
	// 一名活跃的玩家
	Player = 0,

	// Spectator connected to a running game
	// 观众与正在进行的比赛相连接
	LiveSpectator,

	// Spectating a demo recording offline
	// 离线观看演示录制内容
	ReplaySpectator,

	// A deactivated player (disconnected)
	// 一个已停用的玩家（已断线）
	InactivePlayer
};


/**
 * ALyraPlayerState
 *
 *	Base player state class used by this project.
 *	本项目所使用的基础玩家状态类。
 */
UCLASS(MinimalAPI, Config = Game)
class ALyraPlayerState : public AModularPlayerState
{
	GENERATED_BODY()

public:
	/*
	 * 构造函数
	 * 创建GAS组件 开启网路同步 设置同步模式
	 * 创建生命值组件
	 * 创建战斗组件
	 * 设置同步率
	 * 初始化队伍信息
	 */
	UE_API ALyraPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	

	//获取玩家控制器
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerState")
	UE_API ALyraPlayerController* GetLyraPlayerController() const;


};





#undef UE_API