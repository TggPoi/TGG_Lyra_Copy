// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.

#pragma once

#include "CommonGameViewportClient.h"

#include "LyraGameViewportClient.generated.h"


class UGameInstance;
class UObject;

/**
 * UGameViewportClient
 * 游戏视口（FViewport）是一个针对特定平台的渲染、音频和输入子系统的高级抽象接口。
 * GameViewportClient 是引擎与游戏视口之间的接口。
 * 对于每个游戏实例，都会创建一个唯一的 GameViewportClient 实例。最终的译文：这个
 * 目前唯一一种可能的情况是，您可能会有一个引擎实例，但
 * 游戏的多个实例（因此会有多个 GameViewportClients）的情况是当
 * 您同时运行多个 PIE 窗口时才会出现。*
 * 职责：
 * 将输入事件传播至全局交互列表中*
 * @见 UGameViewportClient
 * 
 */

// UCommonGameViewportClient: 常用用户界面视图首先会将输入重新导向用户界面。这是为了使常用用户界面能够进行输入的路由/处理。

// Lyra项目使用的视口类
UCLASS(BlueprintType)
class ULyraGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	ULyraGameViewportClient();

	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};