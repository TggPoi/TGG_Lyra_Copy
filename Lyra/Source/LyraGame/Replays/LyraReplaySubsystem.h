// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Not Finished. 只写了基础类.
#pragma once

#include "NetworkReplayStreaming.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"

#include "LyraReplaySubsystem.generated.h"

#define UE_API LYRAGAME_API

/*
 * 回放系统暂时不讲解
 * 
 */
UCLASS(MinimalAPI)
class ULyraReplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API ULyraReplaySubsystem();

	/** Returns true if this platform supports replays at all */
	/** 返回值为真表示此平台完全支持回放功能 */
	UFUNCTION(BlueprintCallable, Category = Replays, BlueprintPure = false)
	static UE_API bool DoesPlatformSupportReplays();

	/** Returns the trait tag for platform support, used in options */
	/** 返回用于平台支持的特性标签，用于在选项中使用 */
	static UE_API FGameplayTag GetPlatformSupportTraitTag();
};










#undef UE_API



