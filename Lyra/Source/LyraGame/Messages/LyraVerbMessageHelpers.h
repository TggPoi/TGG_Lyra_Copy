// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
// 001 蓝图静态函数库 主要用于便捷获取玩家控制器和玩家状态. 对于Message和Cue的参数进行快速转换.
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "LyraVerbMessageHelpers.generated.h"


#define UE_API LYRAGAME_API

struct FGameplayCueParameters;
struct FLyraVerbMessage;

class APlayerController;
class APlayerState;
class UObject;
struct FFrame;


/*
 * 蓝图静态函数库
 * 主要用于通讯Message的处理
 * UBlueprintFunctionLibrary 中的函数必须是静态的，并且要在Engine模块下使用
 */
UCLASS(MinimalAPI)
class ULyraVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:	
	// 获取玩家状态
	UFUNCTION(BlueprintCallable, Category = "Lyra")
	static UE_API APlayerState* GetPlayerStateFromObject(UObject* Object);

	// 获取玩家控制器
	UFUNCTION(BlueprintCallable, Category = "Lyra")
	static UE_API APlayerController* GetPlayerControllerFromObject(UObject* Object);

	// 将Message转换城Cue参数
	UFUNCTION(BlueprintCallable, Category = "Lyra")
	static UE_API FGameplayCueParameters VerbMessageToCueParameters(const FLyraVerbMessage& Message);

	// 将Cue参数转换城Message
	UFUNCTION(BlueprintCallable, Category = "Lyra")
	static UE_API FLyraVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
	
};




#undef UE_API
