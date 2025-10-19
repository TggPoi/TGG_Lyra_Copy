// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Finished
#pragma once

#include "Engine/StreamableManager.h"


//单播代理
DECLARE_DELEGATE_OneParam(FLyraAssetManagerStartupJobSubstepProgress, float /*NewProgress*/);

/** Handles reporting progress from streamable handles */
/** 处理来自可流式句柄的进度报告 */
struct FLyraAssetManagerStartupJob
{
	//进度代理
	FLyraAssetManagerStartupJobSubstepProgress SubstepProgressDelegate;

	TFunction<void(const FLyraAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)> JobFunc;

	// 任务名称
	FString JobName;
	
	// 任务权重
	float JobWeight;

	//流式加载的时间
	mutable double LastUpdate = 0;

	/** Simple job that is all synchronous */
	/** 简单的同步型任务 */
	// 任务名,任务函数,权重
	FLyraAssetManagerStartupJob(const FString& InJobName, const TFunction<void(const FLyraAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)>& InJobFunc, float InJobWeight)
		: JobFunc(InJobFunc)
		, JobName(InJobName)
		, JobWeight(InJobWeight)
	{}

	/** Perform actual loading, will return a handle if it created one */
	/** 执行实际加载操作，如果创建了处理对象则会返回该处理对象的句柄 */
	TSharedPtr<FStreamableHandle> DoJob() const;

	// 更新进度 没有用到 因为资产管理器里面并没有注册复杂任务
	void UpdateSubstepProgress(float NewProgress) const
	{
		SubstepProgressDelegate.ExecuteIfBound(NewProgress);
	}
	
	// 根据流式加载句柄 没有用到 因为资产管理器里面并没有注册复杂任务
	void UpdateSubstepProgressFromStreamable(TSharedRef<FStreamableHandle> StreamableHandle) const
	{
		//先判断是否绑定了
		if (SubstepProgressDelegate.IsBound())
		{
			// StreamableHandle::GetProgress traverses() a large graph and is quite expensive
			// StreamableHandle::GetProgress 方法会遍历一个庞大的图结构，其执行效率较低。
			double Now = FPlatformTime::Seconds();
			// 比较时间 每16ms去获取下进度并传递出去
			if (LastUpdate - Now > 1.0 / 60)
			{
				SubstepProgressDelegate.Execute(StreamableHandle->GetProgress());
				LastUpdate = Now;
			}
		}
	}
	

	

	
	
};

