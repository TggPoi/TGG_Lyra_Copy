// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
// 001 Not Finished.定义了基本类
// 002 添加了几个方法给给外部LocalPlayer调用,但是没有写实现.
// 003 大部分代码已添加 涉及到Lyra输入的未添加
#pragma once

#include "GameFramework/SaveGame.h"
#include "SubtitleDisplayOptions.h"

#include "UObject/ObjectPtr.h"
#include "LyraSettingsShared.generated.h"


class UObject;
struct FFrame;

class ULyraLocalPlayer;

/**
 * ULyraSettingsShared - The "Shared" settings are stored as part of the USaveGame system, these settings are not machine
 * specific like the local settings, and are safe to store in the cloud - and 'share' them.  Using the save game system
 * we can also store settings per player, so things like controller keybind preferences should go here, because if those
 * are stored in the local settings all users would get them.
 *
 * ULyraSettingsShared - “共享”设置被存储在 USaveGame 系统中，与本地设置不同，这些设置并非针对特定机器的，而且可以安全地存储在云端并进行“共享”。通过使用保存游戏系统，我们还可以为每个玩家存储设置，比如控制器按键绑定偏好就应存放在这里，因为如果这些设置存放在本地设置中，那么所有用户都会获取到它们。
 */
UCLASS()
class ULyraSettingsShared : public ULocalPlayerSaveGame
{
	GENERATED_BODY()

public:

	// 共享设置变更代理 用于控制器绑定获取是否开启力反馈
	DECLARE_EVENT_OneParam(ULyraSettingsShared, FOnSettingChangedEvent, ULyraSettingsShared* Settings);
	FOnSettingChangedEvent OnSettingChanged;


public:

	// 构造函数 绑定本地化变更代理 初始化手柄盲区值
	ULyraSettingsShared();

	/** Creates a temporary settings object, this will be replaced by one loaded from the user's save game */
	/** 创建一个临时的设置对象，该对象将被从用户的游戏存档中加载的设置对象所替换 */
	static ULyraSettingsShared* CreateTemporarySettings(const ULyraLocalPlayer* LocalPlayer);

	
	/** Synchronously loads a settings object, this is not valid to call before login */
	/** 异步加载一个设置对象，此操作在登录之前不可执行 */
	static ULyraSettingsShared* LoadOrCreateSettings(const ULyraLocalPlayer* LocalPlayer);

	DECLARE_DELEGATE_OneParam(FOnSettingsLoadedEvent, ULyraSettingsShared* Settings);

	/** Starts an async load of the settings object, calls Delegate on completion */
	/** 启动对设置对象的异步加载过程，完成时调用委托函数 */
	static bool AsyncLoadOrCreateSettings(const ULyraLocalPlayer* LocalPlayer, FOnSettingsLoadedEvent Delegate);

public:

	// 获取字幕是否开启
	UFUNCTION()
	bool GetSubtitlesEnabled() const { return false; }

public:

	// 获取力反馈
	UFUNCTION()
	bool GetForceFeedbackEnabled() const { return bForceFeedbackEnabled; }

private:
	/** Is force feedback enabled when a controller is being used? */
	/** 当使用控制器时，力反馈功能是否已开启？*/
	UPROPERTY()
	bool bForceFeedbackEnabled = true;
	
};