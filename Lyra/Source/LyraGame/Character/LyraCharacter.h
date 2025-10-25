// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Not Finihed . 只写了构造函数.
#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"

#include "Teams/LyraTeamAgentInterface.h"


#include "LyraCharacter.generated.h"


#define UE_API LYRAGAME_API

/**
 * ALyraCharacter
 *
 *	The base character pawn class used by this project.
 *	Responsible for sending events to pawn components.
 *	New behavior should be added via pawn components when possible.
 *
 * 本项目所使用的基础角色兵种类。
 * 负责向兵种组件发送事件。
 * 在可能的情况下，应通过兵种组件添加新行为。
 *	
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class ALyraCharacter : public AModularCharacter
{
	GENERATED_BODY()

public:
	/*
	 * 构造函数
	 * 1.开启Tick
	 * 2.修改Mesh的Transform
	 * 3.修改移动组件参数
	 * 4.创建PawnExtComp管理组件拓展器
	 * 5.创建生命值组件
	 * 6.创建相机组件
	 *  
	 */
	UE_API ALyraCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//获取玩家控制器
	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	UE_API ALyraPlayerController* GetLyraPlayerController() const ;

	//获取玩家状态类
	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	UE_API ALyraPlayerState* GetLyraPlayerState() const;
	
	
};





#undef UE_API