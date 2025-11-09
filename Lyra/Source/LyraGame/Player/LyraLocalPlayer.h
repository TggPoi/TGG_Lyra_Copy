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
class ULyraLocalPlayer : public UCommonLocalPlayer, public ILyraTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	// 构造函数 无作用
	UE_API ULyraLocalPlayer();

	//~UObject interface
	// 绑定音频设备变更代理
	UE_API virtual void PostInitProperties() override;
	//~End of UObject interface


	//~UPlayer interface

	/**
	 * 	动态地为玩家分配控制器并设置视口。
	 *	触发控制器变更代理
	 * 	
	 * 	@参数  PC - 用于分配给玩家的新玩家控制器
	 */
	UE_API virtual void SwitchController(class APlayerController* PC) override;
	//~End of UPlayer interface

	//~ULocalPlayer interface

	//~End of ULocalPlayer interface
	/**
	 * 为该玩家创建一个角色。
	 * 触发控制器变更
	 * @参数 URL - 玩家加入时所使用的 URL。
	 * @参数 OutError - 若出现错误，则返回错误描述。
	 * @参数 InWorld - 用于生成玩家角色的场景
	 * @返回值 若出现错误则为 False，若成功生成玩家角色则为 True。*
	 * 
	 */
	UE_API virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;

	/**
	 * 被要求初始化在线代表人员
	 * 
	 */
	UE_API virtual void InitOnlineSession() override;
	
	//~ILyraTeamAgentInterface interface
	// 无作用 我们只从PlayerController里面获取队伍信息
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	// 从控制器中获取队伍信息
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	// 获取队伍变更代理
	UE_API virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	
	//~End of ILyraTeamAgentInterface interface
public:
	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	/** 获取此玩家的本地设置，该设置在程序启动时从配置文件中读取，并且始终有效 */
	UFUNCTION()
	UE_API ULyraSettingsLocal* GetLocalSettings() const;

	/** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
	/** 获取此玩家的共享设置，此操作是通过保存游戏系统来实现的，因此在用户登录之后才能确保其准确性 */
	UFUNCTION()
	UE_API ULyraSettingsShared* GetSharedSettings() const;

	/** Starts an async request to load the shared settings, this will call OnSharedSettingsLoaded after loading or creating new ones */
	/** 启动一个异步请求以加载共享设置，加载或创建新设置完成后会调用 OnSharedSettingsLoaded 方法 */
	// 从ULyraGameInstance::HandlerUserInitialized中调用.
	UE_API void LoadSharedSettingsFromDisk(bool bForceLoad = false);


protected:

	// 当设置加载完成后调用,用以存储对象,避免回收,避免重复加载
	UE_API void OnSharedSettingsLoaded(ULyraSettingsShared* LoadedOrCreatedSettings);

	
	// 用于触发音频设备变更的函数
	UE_API void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);

	// 用于音频设备变更完成后的回调函数
	UFUNCTION()
	UE_API void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);


	// 玩家控制器的变更函数,主要用于重新绑定传递队伍信息
	UE_API void OnPlayerControllerChanged(APlayerController* NewController);

	// 向下级传递队伍变更信息
	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);
	
private:
	// 共享设置
	UPROPERTY(Transient)
	mutable TObjectPtr<ULyraSettingsShared> SharedSettings;

	/**
	 * FUniqueNetIdRepl
	 * 对不透明类型 FUniqueNetId 的包装器*
	 * 确保 FUniqueNetId 中的不透明部分在通过网络远程过程调用（RPC）以及角色复制过程中能够得到正确处理/序列化
	 * 
	 * FUniqueNetId
	 * 基于在线标识的用户资料服务的抽象
	 * 该类应保持不可见性
	 *
	 * 用于避免重复加载
	 */
	FUniqueNetIdRepl NetIdForSharedSettings;

	
	// 临时对象 输入映射的上下文 目前没用到
	UPROPERTY(Transient)
	mutable TObjectPtr<const UInputMappingContext> InputMappingContext;

	
	// 用于传递给下级队伍信息的代理
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

	// 上次绑定的玩家控制器
	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;
	
};





#undef UE_API