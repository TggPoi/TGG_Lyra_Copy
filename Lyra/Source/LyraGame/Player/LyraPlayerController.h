// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Not Finished.只写了基础类
// 002 写了一个命令行注册的代码,因为在开发者设置中引用到了,所以先写了.
// 003 基本搞定了,可能还有一些外部调用得类没有实现.
// 004 恢复了从游戏设置获取是否开启记录回放.
// 005 恢复了从共享设置里面绑定设备变更 以及力反馈.
#pragma once


#include "Camera/LyraCameraAssistInterface.h"
#include "CommonPlayerController.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraPlayerController.generated.h"

#define UE_API LYRAGAME_API

struct FGenericTeamId;

class ALyraHUD;
class ALyraPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class ULyraAbilitySystemComponent;
class ULyraSettingsShared;
class UObject;
class UPlayer;
struct FFrame;

/**
 * ALyraPlayerController
 *
 *	The base player controller class used by this project.
 * 本项目所使用的基础玩家控制器类。
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class ALyraPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:

	/*
	 * 构建函数
	 * 设置相机组件类
	 * 设置作弊组件类
	 */
	UE_API ALyraPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//获取玩家状态
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	UE_API ALyraPlayerState* GetLyraPlayerState() const;

	//获取玩家HUD
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	UE_API ALyraHUD* GetLyraHUD() const;

		
	
};

// A player controller used for replay capture and playback
// 用于录制和回放的玩家控制器
UCLASS()
class ALyraReplayPlayerController : public ALyraPlayerController
{
	GENERATED_BODY()
	
	
};


#undef UE_API