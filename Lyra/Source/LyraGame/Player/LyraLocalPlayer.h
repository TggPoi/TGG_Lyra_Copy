// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
// 001 Not Finished.只写了基础类
// 002 基本完成了 还需要定义设置类
// 003 记得在GameInstance里面恢复调用
#pragma once

#include "CommonLocalPlayer.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraLocalPlayer.generated.h"


#define UE_API LYRAGAME_API

struct FGenericTeamId;

class APlayerController;
class UInputMappingContext;
class ULyraSettingsLocal;
class ULyraSettingsShared;
class UObject;
class UWorld;
struct FFrame;
struct FSwapAudioOutputResult;



/**
 * ULyraLocalPlayer
* Lyra项目的本地玩家类
 */
UCLASS(MinimalAPI)
class ULyraLocalPlayer : public UCommonLocalPlayer
{
	GENERATED_BODY()
	
public:
	// 构造函数 无作用
	UE_API ULyraLocalPlayer();


};





#undef UE_API