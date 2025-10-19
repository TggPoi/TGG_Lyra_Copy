// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
// 001  Not Finished. 需要替换资产管理器 需要更新用户设置
// 002 在体验加载完成后调用设置变更
#pragma once

#include "Components/GameStateComponent.h"
#include "LoadingProcessInterface.h"

#include "LyraExperienceManagerComponent.generated.h"

//定义导出宏（不确定是不是工程中某个位置自动实现了，这里还是手动定义下）
#define UE_API LYRAGAME_API

//前向声明一个结构体
namespace UE::GameFeatures { struct FResult; }

class ULyraExperienceDefinition;

// Experience各个阶段的加载代理,是一个多播.
//通过传递ULyraExperienceDefinition的指针，能够获取PawnData，并且获取内部的数据
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLyraExperienceLoaded, const ULyraExperienceDefinition* /*Experience*/);

// 用来表明Experience的加载状态的枚举,整个状态比较复杂,同时包含异步和同步的过程
enum class ELyraExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};

/*
 * 管理体验的游戏状态组件,非常重要
 * 它在GameState的构造函数中创建,开启了网络同步的功能用来传递Experience
 * 
 */
UCLASS(MinimalAPI)
class ULyraExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	
	UE_API ULyraExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	/**
	 * Ends gameplay for this component.
	 * Called from AActor::EndPlay only if bHasBegunPlay is true
	 * 
	 * 结束此组件的游戏进程。
	 * 仅在 bHasBegunPlay 为真时，从 AActor::EndPlay 中调用此函数。
	 * 这个组件不需要实现BeginPlay，因为用不到，当前组件执行的时机是在Gamemode的InitGame函数中，但是结束执行时还是要自己实现结束机制的
	 */
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	//~ILoadingProcessInterface interface
	// Checks to see if this object implements the interface, and if so asks whether or not we should be currently showing a loading screen
	// ---检查该对象是否实现了该接口，如果实现了则询问是否当前应显示加载界面，【就是游戏启动时的loading界面】
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;

	// Tries to set the current experience, either a UI or gameplay one
	// 尝试设置当前的体验，可以是用户界面体验，也可以是游戏体验。
	// ---这里执行的时机一定是在BeginPlay之后，Gamemode找到GameState后发现这个组件，然后跳转过来设置
	UE_API void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	// Ensures the delegate is called once the experience has been loaded,
	// before others are called.
	// However, if the experience has already loaded, calls the delegate immediately.
	// 确保在体验加载完成后调用该委托函数，
	// 而不会在其他函数被调用之前再调用它。
	// 但若体验已加载完成，则会立即调用该委托函数。
	UE_API void CallOrRegister_OnExperienceLoaded_HighPriority(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	// 确保在体验加载完成后调用委托函数
	// 如果体验已经加载完成，则立即调用该委托函数
	UE_API void CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	// 确保在体验加载完成后调用委托函数
	// 如果体验已经加载完成，则立即调用该委托函数
	UE_API void CallOrRegister_OnExperienceLoaded_LowPriority(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	// This returns the current experience if it is fully loaded, asserting otherwise
	// (i.e., if you called it too soon)
	// 此函数会返回当前的体验状态，如果该体验已完全加载则返回该状态，否则会抛出错误（即，如果您过早调用此函数的话）
	UE_API const ULyraExperienceDefinition* GetCurrentExperienceChecked() const;

	// Returns true if the experience is fully loaded
	// 若体验已完全加载，则返回 true
	UE_API bool IsExperienceLoaded() const;

private:
	//由网络同步过来的Experience从而启动加载,这是客户端的Experience加载启动如果
	UFUNCTION()
	void OnRep_CurrentExperience();
	
	//开始加载
	void StartExperienceLoad();

	//加载完成
	void OnExperienceLoadComplete();

	//当一个GameFeature插件加载完毕,从而减少需要加载GameFeature插件计数.在Experience加载过程中用于计数
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);

	//当Experience完全加载完毕时,需要开启对应的Action列表,并在Action列表执行完毕后,启动之前注册得高中低优先级代理,最后重置用户设置.
	void OnExperienceFullLoadCompleted();

	//当Experience退出时,需要卸载对应的Action,其中一个卸载完成时,增加观察计数.
	void OnActionDeactivationCompleted();
	
	//当Experience退出时,所有Action都卸载后,对后续内容进行处理,比如垃圾回收,卸载等.
	void OnAllActionsDeactivated();

	
private:

	//当前正在使用的体验
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const ULyraExperienceDefinition> CurrentExperience;

	//目前体验的工作状态
	ELyraExperienceLoadState LoadState = ELyraExperienceLoadState::Unloaded;

	//正在加载的游戏特性插件数
	int32 NumGameFeaturePluginsLoading = 0;

	//游戏特性插件对应的URL数组
	TArray<FString> GameFeaturePluginURLs;

	
	//观察到的停留数,用于Action计数
	int32 NumObservedPausers = 0;
	//期望的停留数,用于Action计数
	int32 NumExpectedPausers = 0;
	

	/**
	 * Delegate called when the experience has finished loading just before others
	 * (e.g., subsystems that set up for regular gameplay)
	 */
	/**
	 * 当体验在其他部分加载完成之前就已经完成加载时会触发此委托。
	 * （例如，那些为常规游戏流程做准备的子系统)
	*/
	FOnLyraExperienceLoaded OnExperienceLoaded_HighPriority;

	/** Delegate called when the experience has finished loading */
	/** 当体验加载完成时所调用的委托函数 */
	FOnLyraExperienceLoaded OnExperienceLoaded;

	/** Delegate called when the experience has finished loading */
	/** 当体验加载完成时所调用的委托函数 */
	FOnLyraExperienceLoaded OnExperienceLoaded_LowPriority;
	
};





#undef UE_API