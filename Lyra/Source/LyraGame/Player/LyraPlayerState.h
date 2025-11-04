// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Not Finished.只写了基础类
// 002 还需要等待Charccter初始化过程,以及创建ASC的属性集的定义
#pragma once

#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "System/GameplayTagStack.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraPlayerState.generated.h"

#define UE_API LYRAGAME_API

struct FLyraVerbMessage;

class AController;
class ALyraPlayerController;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class ULyraAbilitySystemComponent;
class ULyraExperienceDefinition;
class ULyraPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;


/** Defines the types of client connected */
/** 定义已连接客户端的类型 */
UENUM()
enum class ELyraPlayerConnectionType : uint8
{
	// An active player
	// 一名活跃的玩家
	Player = 0,

	// Spectator connected to a running game
	// 观众与正在进行的比赛相连接
	LiveSpectator,

	// Spectating a demo recording offline
	// 离线观看演示录制内容
	ReplaySpectator,

	// A deactivated player (disconnected)
	// 一个已停用的玩家（已断线）
	InactivePlayer
};


/**
 * ALyraPlayerState
 *
 *	Base player state class used by this project.
 *	本项目所使用的基础玩家状态类。
 */
UCLASS(MinimalAPI, Config = Game)
class ALyraPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public ILyraTeamAgentInterface
{
	GENERATED_BODY()

public:
	/*
	 * 构造函数
	 * 创建GAS组件 开启网路同步 设置同步模式
	 * 创建生命值组件
	 * 创建战斗组件
	 * 设置同步率
	 * 初始化队伍信息
	 */
	UE_API ALyraPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//获取玩家控制器
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerState")
	UE_API ALyraPlayerController* GetLyraPlayerController() const;

	// 获取 LyraASC 该组件通过构造函数创建
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerState")
	ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const { return AbilitySystemComponent; }
	// 这是IAbilitySystemInterface的重写方法 返回成员变量即可
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//要发送的事件名称
	static UE_API const FName NAME_LyraAbilityReady;

	// 获取PawnData
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	// 在[服务器]得Experience加载完成之后,设置PawnData,然后通过网络同步到客户端
	UE_API void SetPawnData(const ULyraPawnData* InPawnData);
	
	//~AActor interface
	// 执行父类 暂无作用
	UE_API virtual void PreInitializeComponents() override;
	// 执行父类 初始化ASC的ActorInfo 然后绑定体验加载完成回调
	UE_API virtual void PostInitializeComponents() override;
	//~End of AActor interface

	
	//~APlayerState interface 也可以使用 #pragma region 和 #pragma endregion 来分组
	/** 将角色重置至初始状态 - 用于在不重新加载的情况下重新开始关卡时使用。*/
	UE_API virtual void Reset() override;
	
	/** 当控制器的玩家状态首次进行复制时，会调用此方法。*/
	UE_API virtual void ClientInitialize(AController* C) override;

	/** 复制需要保存在非活动玩家状态中的属性 */
	UE_API virtual void CopyProperties(APlayerState* PlayerState) override;
	
	/** 当拥有该玩家状态的玩家断开连接时，服务器会调用此方法，通常情况下此方法会销毁该玩家状态 */
	// 计划添加到体验中 是否开启支持自动摧毁玩家状态
	UE_API virtual void OnDeactivated() override;
	
	/** 当拥有该玩家状态的玩家重新连接后，服务器会调用此函数，并将此玩家状态添加到活跃玩家数组中 */
	// 切换到玩家链接为活跃状态
	UE_API virtual void OnReactivated() override;
	
	//~End of APlayerState interface

	//~ILyraTeamAgentInterface interface
	// 设置队伍ID
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	// 获取队伍ID
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface

	// 设置玩家链接类型
	UE_API void SetPlayerConnectionType(ELyraPlayerConnectionType NewType);
	
	// 获取玩家链接类型
	ELyraPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }
	
	/** Returns the Squad ID of the squad the player belongs to. */
	/** 返回玩家所属小队的编号。*/
	UFUNCTION(BlueprintCallable)
	int32 GetSquadId() const
	{
		return MySquadID;
	}

	/** Returns the Team ID of the team the player belongs to. */
	/** 返回玩家所属团队的团队 ID 。*/
	UFUNCTION(BlueprintCallable)
	int32 GetTeamId() const
	{
		return GenericTeamIdToInteger(MyTeamID);
	}
	// 设置小队编号
	UE_API void SetSquadID(int32 NewSquadID);

	/*
	 * Send a message to just this player
	 * (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	 * 向仅此一位玩家发送消息
	 * 仅用于客户端通知，例如荣誉奖励、任务庆祝信息等，这类信息偶尔丢失也是可以接受的）
	 *
	 * 服务器调用,客户端执行
	 */
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "Lyra|PlayerState")
	UE_API void ClientBroadcastMessage(const FLyraVerbMessage Message);

	// Gets the replicated view rotation of this player, used for spectating
	// 获取此玩家的复制视图旋转角度，用于旁观模式使用
	UE_API FRotator GetReplicatedViewRotation() const;

	// Sets the replicated view rotation, only valid on the server
	// 设置复制视图的旋转角度，仅在服务器端有效 由控制器从PlayerTick中调用
	UE_API void SetReplicatedViewRotation(const FRotator& NewRotation);


private:
	/*
	 * 在初始化的过程中绑定执行等待体验加载完毕后,读取设置PawnData
	 * PostInitializeComponents中绑定
	 * 
	 */
	UE_API void OnExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience);

	
protected:

	// 暂无功能
	UFUNCTION()
	UE_API void OnRep_PawnData();
	
protected:

	// 客户端这个变量需要从网络同步过来
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const ULyraPawnData> PawnData;

	
private:
	
	// The ability system component sub-object used by player characters.
	// 用于玩家角色的“能力系统”子组件。
	UPROPERTY(VisibleAnywhere, Category = "Lyra|PlayerState")
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;
	
	// Health attribute set used by this actor.
	// 该角色所使用的健康属性设定。
	/*UPROPERTY()
	TObjectPtr<const class ULyraHealthSet> HealthSet;*/
	
	// Combat attribute set used by this actor.
	// 此角色所使用的战斗属性设定。
	/*UPROPERTY()
	TObjectPtr<const class ULyraCombatSet> CombatSet;*/

	
	// 玩家链接的类型需要进行网络同步
	UPROPERTY(Replicated)
	ELyraPlayerConnectionType MyPlayerConnectionType;
	
	// 队伍发生改变的代理
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;
	
	// 队伍ID
	UPROPERTY(ReplicatedUsing=OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	// 子战队ID
	UPROPERTY(ReplicatedUsing=OnRep_MySquadID)
	int32 MySquadID;

	// 用于观战的同步角度
	UPROPERTY(Replicated)
	FRotator ReplicatedViewRotation;
	
private:
	// 通知队伍发生了改变
	UFUNCTION()
	UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);

	// 暂无功能
	UFUNCTION()
	UE_API void OnRep_MySquadID();

};





#undef UE_API