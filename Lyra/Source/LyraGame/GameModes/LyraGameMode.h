// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Not Finihed . 只写了构造函数,并指定游戏基础类.
// 002 基本完成了 留了三处需要补充的 玩家控制器,玩家状态类和机器人控制器
#pragma once

#include "ModularGameMode.h"

#include "LyraGameMode.generated.h"

#define UE_API LYRAGAME_API

class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class UClass;
class ULyraExperienceDefinition;
class ULyraPawnData;
class UObject;
struct FFrame;
struct FPrimaryAssetId;
enum class ECommonSessionOnlineMode : uint8;

/**
 * Post login event, triggered when a player or bot joins the game as well as after seamless and non seamless travel
 *
 * 登录后事件，当玩家或机器人加入游戏时触发，以及在无缝和非无缝移动之后触发。
 *
 * This is called after the player has finished initialization
 * 这是在玩家完成初始化操作之后所执行的程序。
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLyraGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);



/**
 * ALyraGameMode
 *
 *	The base game mode class used by this project.
 *	该项目所使用的基础游戏模式类。
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ALyraGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:

	//构造函数 用于指定项目使用的默认类
	UE_API ALyraGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

};



#undef UE_API
