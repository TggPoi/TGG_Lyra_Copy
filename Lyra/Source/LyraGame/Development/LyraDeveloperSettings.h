// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/SoftObjectPath.h"
#include "LyraDeveloperSettings.generated.h"


struct FPropertyChangedEvent;

class ULyraExperienceDefinition;


/*
 * 作弊执行时机
 */
UENUM()
enum class ECheatExecutionTime
{
	// When the cheat manager is created
	// 当作弊管理器被创建时
	OnCheatManagerCreated,

	// When a pawn is possessed by a player
	// 当一个兵卒被玩家控制时
	OnPlayerPawnPossession
};


/*
 * 需要执行的作弊描述指令的结构体
 */
USTRUCT()
struct FLyraCheatToRun
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ECheatExecutionTime Phase = ECheatExecutionTime::OnPlayerPawnPossession;

	UPROPERTY(EditAnywhere)
	FString Cheat;
};


/**
 * Developer settings / editor cheats
 * 开发者设置 / 编辑器作弊功能
 */
UCLASS(config=EditorPerProjectUserSettings, MinimalAPI)
class ULyraDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	
	ULyraDeveloperSettings();

	//~UDeveloperSettings interface
	// 标识类别
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface
	
public:
	// The experience override to use for Play in Editor (if not set, the default for the world settings of the open map will be used)
	// 在编辑器中用于游戏运行的体验设置（若未进行设置，则将使用当前打开地图的世界设置的默认值）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=Lyra, meta=(AllowedTypes="LyraExperienceDefinition"))
	FPrimaryAssetId ExperienceOverride;

	// 是否重写机器人数量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=LyraBots, meta=(InlineEditConditionToggle))
	bool bOverrideBotCount = false;

	// 重写的机器人数量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=LyraBots, meta=(EditCondition=bOverrideBotCount))
	int32 OverrideNumPlayerBotsToSpawn = 0;

	// 是否允许机器人攻击 行为树中的蓝图节点控制
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=LyraBots)
	bool bAllowPlayerBotsToAttack = true;


	// Do the full game flow when playing in the editor, or skip 'waiting for player' / etc... game phases?
	// 在编辑器中进行游戏时，是否要完整执行游戏流程，还是可以跳过“等待玩家”/等等等游戏阶段？
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=Lyra)
	bool bTestFullGameFlowInPIE = false;

	/**
	* Should force feedback effects be played, even if the last input device was not a gamepad?
	* The default behavior in Lyra is to only play force feedback if the most recent input device was a gamepad.
	*
	* 如果要播放力反馈效果的话，即便上一个输入设备并非游戏手柄呢？
	* 在莱拉中，默认设置是仅在最近的输入设备为游戏手柄的情况下才播放力反馈效果。
	* 
	*/
	UPROPERTY(config, EditAnywhere, Category = Lyra, meta = (ConsoleVariable = "LyraPC.ShouldAlwaysPlayForceFeedback"))
	bool bShouldAlwaysPlayForceFeedback = false;


	// Should game logic load cosmetic backgrounds in the editor or skip them for iteration speed?
	// 在编辑器中，游戏逻辑应加载外观装饰背景还是跳过这部分以加快迭代速度？
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=Lyra)
	bool bSkipLoadingCosmeticBackgroundsInPIE = false;

	// List of cheats to auto-run during 'play in editor'
	// 在“编辑器中游玩”模式下自动运行的作弊选项列表
	UPROPERTY(config, EditAnywhere, Category=Lyra)
	TArray<FLyraCheatToRun> CheatsToRun;
	
	// Should messages broadcast through the gameplay message subsystem be logged?
	// 在游戏玩法消息子系统中广播的消息是否需要进行记录？
	UPROPERTY(config, EditAnywhere, Category=GameplayMessages, meta=(ConsoleVariable="GameplayMessageSubsystem.LogMessages"))
	bool LogGameplayMessages = false;
	
#if WITH_EDITORONLY_DATA
	
	/** A list of common maps that will be accessible via the editor detoolbar */
	/** 一系列常见的地图列表，可通过编辑器工具栏进行访问 */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category=Maps, meta=(AllowedClasses="/Script/Engine.World"))
	TArray<FSoftObjectPath> CommonEditorMaps;
#endif

#if WITH_EDITOR
public:
	// Called by the editor engine to let us pop reminder notifications when cheats are active
	// 由编辑引擎调用，以便在作弊功能激活时向我们发送提醒通知
	LYRAGAME_API void OnPlayInEditorStarted() const;

private:
	// 应用设置 目前没有使用 因为这里的设置都是外部读取 不需要自己去主动更新
	void ApplySettings();
#endif

public:
	//~UObject interface
#if WITH_EDITOR
	/**
	* 当此对象上的某个属性被外部修改时触发此事件*
	* @参数 改变的属性  被修改的属性
	* 
	*/
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	/**
	 * 在对象重新加载其配置数据之后，从“ReloadConfig”函数中被调用。
	 */
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
	/**
	 * 在 C++ 构造函数之后、属性（包括从配置文件加载的属性）初始化完成之后调用。
	 * 此函数在任何序列化操作或其他设置操作尚未进行之前被调用。
	 * 
	 */
	virtual void PostInitProperties() override;
#endif
	//~End of UObject interface

	
	

};