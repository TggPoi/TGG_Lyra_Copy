// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "AsyncAction_ExperienceReady.generated.h"

class AGameStateBase;
class ULyraExperienceDefinition;
class UWorld;
struct FFrame;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExperienceReadyAsyncDelegate);

/**
 * Asynchronously waits for the game state to be ready and valid and then calls the OnReady event.  Will call OnReady
 * immediately if the game state is valid already.
 *
 * 异步等待游戏状态准备好且有效，然后调用“就绪”事件。  如果游戏状态已经有效，则会立即调用“就绪”事件。
 *
 * 如果要实现一个异步任务，一般不会把任务绑定在U类或者A类
 * U类的生命周期太复杂，并且会创建CDO对象，增加运行负担；A类生命周期和场景绑定，切换场景容易出问题
 * 推荐做法是创建一个插件单独处理异步任务，然后在需要的时候调用这个插件的异步任务，通过第三方库封装的方式
 */


UCLASS()
class UAsyncAction_ExperienceReady : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()
	
public:
	// Waits for the experience to be determined and loaded
	// 等待体验结果被确定并加载完成
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UAsyncAction_ExperienceReady* WaitForExperienceReady(UObject* WorldContextObject);

	// 自动激活,由蓝图调用.如果是C++,请自行调用
	virtual void Activate() override;

	
public:

	// Called when the experience has been determined and is ready/loaded
	// 当体验内容已确定并准备就绪/加载完成时触发此事件
	UPROPERTY(BlueprintAssignable)
	FExperienceReadyAsyncDelegate OnReady;


private:
	// 步骤1 设置GameState
	void Step1_HandleGameStateSet(AGameStateBase* GameState);

	// 步骤2 绑定事件
	void Step2_ListenToExperienceLoading(AGameStateBase* GameState);
	
	// 步骤3 处理Experience加载完毕
	void Step3_HandleExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience);

	// 步骤四 广播并销毁自身
	void Step4_BroadcastReady();
	

	TWeakObjectPtr<UWorld> WorldPtr;

};