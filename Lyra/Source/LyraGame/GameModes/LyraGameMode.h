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

	/*
	 * 从控制器拿到PawnData
	 * 可以AI控制器,也可以是玩家控制器
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|Pawn")
	UE_API const ULyraPawnData* GetPawnDataForController(const AController* InController) const;


	
	//~AGameModeBase interface
	/**
	 * 初始化游戏。
	 * 在调用任何其他函数（包括 PreInitializeComponents() 函数）之前，会调用 GameMode 的 InitGame() 事件。
	 * 这个事件由 GameMode 使用来初始化参数并生成其辅助类。
	 * 注意：此事件在角色的 PreInitializeComponents 事件之前被调用。
	 */
	UE_API virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;


	/**
	 * 返回给定控制器的默认兵类
	 * 我们重写了这个方法,当我们加载体验完成之后PawnData就应该可用了.通过它获取
	 * 初始化的时候,可能没有加载完成,那就现用默认的
	 * 
	 */
	UE_API virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	
	/**
	 * 重写这个函数是为了在生成过程中将PawnData的数据注入，（在出生地playerstart 和使用的类型确定了之后，会执行这里开始spawn
	 * 在“重启玩家”过程中调用此函数以实际生成玩家的兵种，当使用变换时
	 * @参数 新玩家 - 生成兵种的控制器
	 * @参数 生成变换 - 用于生成兵种的变换体
	 * @返回 一个默认兵种类的兵种
	 * 
	 */
	UE_API virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	
	/** 如果“查找玩家起点”操作应使用玩家身上所保存的“起点位置”而非调用“选择玩家起点”功能，则返回 true */
	UE_API virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	
	/** 表示玩家已准备好进入游戏，此时游戏可能会启动 */
	UE_API virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	/**
	 * 返回此玩家可从何处开始生成的“最佳”位置
	 * 默认实现会寻找一个随机且未被占用的位置*
	 * @参数 Player：我们为它所选择的控制角色
	 * @返回值：被选作玩家起始位置的 AActor 对象（通常为 PlayerStart 类型）
	 * 转发给LyraPlayerSpawningManagerComponent
	 *	 
	 */
	UE_API virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	

	/**
	 * 处理重新播放玩家的后半部分内容
	 * 转发给LyraPlayerSpawningManagerComponent
	 */
	UE_API virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

	/**
	 * 若调用“重启玩家”操作是有效的，则返回 true。默认情况下会调用“玩家”的“可以重启玩家”方法
	 * 例如设置一个机器人只有3条命，3次死亡后就不能重启
	 * 转发给LyraPlayerSpawningManagerComponent
	 */
	UE_API virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	
	// 绑定体验加载完成之后回调
	UE_API virtual void InitGameState() override;


	/**
	 * 尝试初始化玩家的“起始位置”。
	 * @参数  Player  玩家的控制器。
	 * @参数  OutErrorMessage  任何错误信息。
	 * @返回  bool  如果更新了起始位置则返回 true，否则返回 false。
	 * 在GameModeBase的AGameModeBase::Login->InitNewPlayer()调用.时机太早了.我们用不上
	 * 
	 */
	UE_API virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;

	/**
	 * 负责所有在旅行方式之间共享的玩家初始化工作
	 * （即从 PostLogin() 和 HandleSeamlessTravelPlayer() 中调用）*
	 * 
	 */
	UE_API virtual void GenericPlayerInitialization(AController* NewPlayer) override;

	UE_API virtual void FailedToRestartPlayer(AController* NewPlayer) override;


	// Restart (respawn) the specified player or bot next frame
	// - If bForceReset is true, the controller will be reset this frame (abandoning the currently possessed pawn, if any)
	UFUNCTION(BlueprintCallable)
	UE_API void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);


	
	//~End of AGameModeBase interface
	
	/*
	 * Agnostic version of PlayerCanRestart that can be used for both player bots and players
	 * “无神论版”的“玩家可重置”功能，可用于玩家机器人和玩家自身。
	 * 转发给LyraPlayerSpawningManagerComponent
	 * 
	 */
	UE_API virtual bool ControllerCanRestart(AController* Controller);


	// Delegate called on player initialization, described above
	// 在玩家初始化时调用的委托函数，如上文所述
	FOnLyraGameModePlayerInitialized OnGameModePlayerInitialized;



protected:
	/*
	 * 体验加载完成后回调
	 * 它的调用可能晚于原引擎的玩家生成流程,所以需要重设玩家状态
	 * 在InitGameState中绑定.
	 */
	UE_API void OnExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience);
	// 体验是否加载完成 通过访问GameState的体验管理插件得知
	UE_API bool IsExperienceLoaded() const;

	
	/*
	 * 确定体验之后调用GameState的体验管理组件,在服务端设置体验,开始体验的加载流程.
	 * 
	 */
	UE_API void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);


	
	/*
	 * 重载体验experience 的选型
	 * 在InitGame的绑定到下一帧调用执行
	 * 包含两种路径,一种是用户自己拉起来游戏,还有一种是DS举行在线服务.
	 * 
	 */
	UE_API void HandleMatchAssignmentIfNotExpectingOne();


	// 专属服务器情况开始自动登录 等待登录结果 去触发体验重写开始主持游戏
	UE_API bool TryDedicatedServerLogin();
	UE_API void HostDedicatedServerMatch(ECommonSessionOnlineMode OnlineMode);

	/*
	 * 专属服务器情况下,本地用户注册登录的结果回调
	 */
	UFUNCTION()
	UE_API void OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo,
		bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);
	

};



#undef UE_API