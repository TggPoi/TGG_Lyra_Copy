// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
// 001 Not Finished.只写了基础类
// 002 基本完成了.但是有些变量和函数,目前用不到.后面会单独抽几章去讲.比如GAS相关和回放相关的.
#pragma once

#include "AbilitySystemInterface.h"
#include "ModularGameState.h"

#include "LyraGameState.generated.h"

#define UE_API LYRAGAME_API

struct FLyraVerbMessage;

class APlayerState;
class UAbilitySystemComponent;
class ULyraAbilitySystemComponent;
class ULyraExperienceManagerComponent;
class UObject;
struct FFrame;

/**
 * ALyraGameState
 *
 *	The base game state class used by this project.
 * 本项目所使用的基础游戏状态类。
 */
UCLASS(MinimalAPI, Config = Game)
class ALyraGameState : public AModularGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	/*
	 * 构造函数
	 * 1.开启Tick的功能
	 * 2.初始化GAS组件 设置GAS的同步模式
	 * 3.创建体验管理组件 负载加载Experience
	 */
	UE_API ALyraGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface

	// 暂无功能
	UE_API virtual void PreInitializeComponents() override;
	
	// 暂无功能
	UE_API virtual void PostInitializeComponents() override;
	
	// 暂无功能
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// 从服务端读取帧数通过属性同步到客户端
	UE_API virtual void Tick(float DeltaSeconds) override;
	//~End of AActor interface
	
	//~IAbilitySystemInterface
	// 获取ASC组件
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface
	
	// Gets the ability system component used for game wide things
	// 获取用于全局游戏操作的能力系统组件（由蓝图调用）
	UFUNCTION(BlueprintCallable, Category = "Lyra|GameState")
	ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const { return AbilitySystemComponent; }

	

	//~AGameStateBase interface
	/** 将玩家状态添加到玩家数组中  暂无功能*/
	UE_API virtual void AddPlayerState(APlayerState* PlayerState) override;
	
	/** 从玩家数组中移除玩家状态信息。暂无功能 */
	UE_API virtual void RemovePlayerState(APlayerState* PlayerState) override;
	
	/**
	 * 在无缝旅行过渡过程中被调用两次（一次是在过渡地图加载时，一次是在目的地地图加载时）
	 * 主要用于移除无效的用户和机器人
	 * 
	 */
	UE_API virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;

	//~End of AGameStateBase interface
	
public:
	// Send a message that all clients will (probably) get
	// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	// 发送一条消息，告知所有客户端（很可能）都会接收到该消息
	// （仅用于客户端通知，例如淘汰通知、服务器加入消息等，这类消息可以丢失）
	// 服务器调用,所有客户端执行
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "Lyra|GameState")
	UE_API void MulticastMessageToClients(const FLyraVerbMessage Message);

	// Send a message that all clients will be guaranteed to get
	// (use only for client notifications that cannot handle being lost)
	// 发送一条消息，确保所有客户端都能接收到该消息
	// （仅用于那些无法承受消息丢失情况的客户端通知）
	// 服务器调用 所有客户端执行
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Lyra|GameState")
	UE_API void MulticastReliableMessageToClients(const FLyraVerbMessage Message);


	
	// Gets the server's FPS, replicated to clients
	// 获取服务器的帧率，这是由服务器同步过来的
	UE_API float GetServerFPS() const;


	
	
	// Indicate the local player state is recording a replay
	// 表示本地玩家状态正在录制回放
	UE_API void SetRecorderPlayerState(APlayerState* NewPlayerState);

	// Gets the player state that recorded the replay, if valid
	// 获取记录回放时所使用的玩家状态（如果该状态有效）
	UE_API APlayerState* GetRecorderPlayerState() const;

	// Delegate called when the replay player state changes
	// 当回放播放器状态发生变化时触发的回调函数 在玩家控制器里处理
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRecorderPlayerStateChanged, APlayerState*);
	FOnRecorderPlayerStateChanged OnRecorderPlayerStateChangedEvent;

	
private:
	
	// Handles loading and managing the current gameplay experience
	// 负责加载并管理当前的游戏体验
	UPROPERTY()
	TObjectPtr<ULyraExperienceManagerComponent> ExperienceManagerComponent;


	// The ability system component subobject for game-wide things (primarily gameplay cues)
	// 用于游戏范围内各类事物（主要是游戏提示）的能力系统子对象
	UPROPERTY(VisibleAnywhere, Category = "Lyra|GameState")
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;




	
protected:
	
	UPROPERTY(Replicated)
	float ServerFPS;


	// The player state that recorded a replay, it is used to select the right pawn to follow
	// This is only set in replay streams and is not replicated normally
	// 玩家所记录的回放状态，用于选择正确的棋子进行跟随
	// 这种状态仅在回放流中设置，通常不会进行复制操作
	UPROPERTY(Transient, ReplicatedUsing = OnRep_RecorderPlayerState)
	TObjectPtr<APlayerState> RecorderPlayerState;

	// 用于属性网络同步过来之后进行处理.
	UFUNCTION()
	UE_API void OnRep_RecorderPlayerState();

	
	
};



#undef UE_API