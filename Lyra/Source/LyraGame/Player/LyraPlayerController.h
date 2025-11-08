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
class ALyraPlayerController : public ACommonPlayerController, public ILyraCameraAssistInterface, public ILyraTeamAgentInterface
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
	
	// 获取ASC组件
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	UE_API ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const;
	
	//获取玩家HUD
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	UE_API ALyraHUD* GetLyraHUD() const;

	// Call from game state logic to start recording an automatic client replay if ShouldRecordClientReplay returns true
	// 在游戏状态逻辑中，如果“应录制客户端回放”这一条件返回为真，则启动自动录制客户端回放的过程
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	UE_API bool TryToRecordClientReplay();

	// Call to see if we should record a replay, subclasses could change this
	// 调用此方法以确定是否需要录制回放，子类可以对此进行修改
	UE_API virtual bool ShouldRecordClientReplay();

	// Run a cheat command on the server.
	// 在服务器上运行作弊指令。
	UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheat(const FString& Msg);

	// Run a cheat command on the server for all players.
	// 在服务器上为所有玩家执行作弊指令。
	UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheatAll(const FString& Msg);
	
#pragma region AActor
	//~AActor interface
	// 暂无功能
	UE_API virtual void PreInitializeComponents() override;

	// 注册一个HTTP Server的回调
	UE_API virtual void BeginPlay() override;

	// 暂无功能
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 用来同步TargetViewRotation
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	
	//~End of AActor interface
#pragma endregion


#pragma region AController

	//~AController interface
	// 嵌入作弊时机
	UE_API virtual void OnPossess(APawn* InPawn) override;

	// 确保ASC的解除成功
	UE_API virtual void OnUnPossess() override;

	// 初始化PlayerState 并广播出去 在广播中绑定到了PlayerState的队伍变更
	UE_API virtual void InitPlayerState() override;

	// 清理PlayerState 并广播出去
	UE_API virtual void CleanupPlayerState() override;

	// 确保专属服务器下的ASC执行时序问题
	UE_API virtual void OnRep_PlayerState() override;

	
	
	//~End of AController interface

#pragma endregion 

#pragma region APlayerController
	//~APlayerController interface
	// 暂无功能
	UE_API virtual void ReceivedPlayer() override;
	
	// 确保TargetViewRotation的获取正确
	UE_API virtual void PlayerTick(float DeltaTime) override;

	// 绑定本地玩家类的变更,从而响应设置的更改.
	UE_API virtual void SetPlayer(UPlayer* InPlayer) override;

	// 是否开启作弊
	UE_API virtual void AddCheats(bool bForce) override;

	// 更新力反馈
	UE_API virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;

	/**
	 * 根据游戏玩法构建一个隐藏组件的列表
	 * @参数 视图位置：需要隐藏/取消隐藏的视点
	 * @参数 隐藏组件列表：要添加到列表中或从列表中移除的组件列表
	 * 
	 */	
	UE_API virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
	
	// 暂无功能
	UE_API virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;

	// 这里去主动激活ASC组件的输入事件
	UE_API virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	
	//~End of APlayerController interface
#pragma endregion


	//~ILyraCameraAssistInterface interface
	// 当被相机穿透的时候需要调用此函数去设置一帧的隐藏
	UE_API virtual void OnCameraPenetratingTarget() override;
	//~End of ILyraCameraAssistInterface interface

	
#pragma region ILyraTeamAgentInterface
	//~ILyraTeamAgentInterface interface
	
	// 设置队伍,此函数不应该被主动调用,因为GenericTeamId 是由PlayerState来驱动修改的.
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	// 从PlayerState获取队伍ID
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;

	// 用于绑定,主要是Character来这里进行绑定
	UE_API virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	

	//~End of ILyraTeamAgentInterface interface
#pragma endregion
	
	// 开启自动运行
	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	UE_API void SetIsAutoRunning(const bool bEnabled);

	// 获取是否自动运行
	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	UE_API bool GetIsAutoRunning() const;
	

private:
	
	// 获取切换的代理,用于下级单位进行绑定,如Character
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

	// 记录上次看见的PlayerState
	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;
	
private:
	// 用于绑定在PlayerState上的队伍接口,用于在PlayerState的队伍变更时进行响应
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
	// Called when the player state is set or cleared
	// 当玩家状态被设置或取消时触发此事件
	UE_API virtual void OnPlayerStateChanged();

	
	// 广播玩家状态发生了变化,需要重新绑定队伍代理
	void BroadcastOnPlayerStateChanged();
	
protected:

	//~APlayerController interface

	//~End of APlayerController interface
	
	// 用于绑定到本地玩家上面的设置,实现读取是否开启力反馈
	UE_API void OnSettingsChanged(ULyraSettingsShared* InSettings);


	
	// 开启自动运行
	UE_API void OnStartAutoRun();
	// 关闭自动运行
	UE_API void OnEndAutoRun();
	
protected:
	
	// 蓝图拓展事件
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnStartAutoRun"))
	UE_API void K2_OnStartAutoRun();

	// 蓝图拓展事件
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnEndAutoRun"))
	UE_API void K2_OnEndAutoRun();

	// 用于被相机穿透时去隐藏一帧 使用后即复原
	bool bHideViewTargetPawnNextFrame = false;

	
	
};

// A player controller used for replay capture and playback
// 用于录制和回放的玩家控制器
UCLASS()
class ALyraReplayPlayerController : public ALyraPlayerController
{
	GENERATED_BODY()

	virtual void Tick(float DeltaSeconds) override;
	virtual void SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds) override;
	virtual bool ShouldRecordClientReplay() override;

	// Callback for when the game state's RecorderPlayerState gets replicated during replay playback
	// 当在回放过程中游戏状态的“记录玩家状态”被复制时的回调函数
	void RecorderPlayerStateUpdated(APlayerState* NewRecorderPlayerState);

	// Callback for when the followed player state changes pawn
	// 当关注的玩家状态发生变化时（例如角色状态改变）的回调函数
	UFUNCTION()
	void OnPlayerStatePawnSet(APlayerState* ChangedPlayerState, APawn* NewPlayerPawn, APawn* OldPlayerPawn);
	
	// The player state we are currently following */
	// 我们当前所遵循的玩家状态 */
	UPROPERTY(Transient)
	TObjectPtr<APlayerState> FollowedPlayerState;
	
};





#undef UE_API