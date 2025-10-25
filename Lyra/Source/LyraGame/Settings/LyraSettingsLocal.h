// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Not Finished.只写了基础类
// 002 写了三个命令行注册的代码.因为在平台模拟设置中用到了,所以先写了.
// 003 添加了一个音频设备切换的代理
// 004 添加了一个Experience加载后调用方法
#pragma once

#include "GameFramework/GameUserSettings.h"
#include "InputCoreTypes.h"

#include "LyraSettingsLocal.generated.h"

enum class ECommonInputType : uint8;
enum class ELyraDisplayablePerformanceStat : uint8;
enum class ELyraStatDisplayMode : uint8;

class ULyraLocalPlayer;
class UObject;
class USoundControlBus;
class USoundControlBusMix;
struct FFrame;



/**
 * UGameUserSettings:
 * Stores user settings for a game (for example graphics and sound settings), with the ability to save and load to and from a file.
 *
 * 为游戏保存用户设置（例如图形和声音设置），并具备将设置保存至文件、从文件加载设置以及从文件中加载并应用设置的功能。
 * 
 */
/**
 * ULyraSettingsLocal
 */

UCLASS()
class ULyraSettingsLocal : public UGameUserSettings
{
	GENERATED_BODY()

public:

	// 构造函数
	ULyraSettingsLocal();

	static ULyraSettingsLocal* Get();

};