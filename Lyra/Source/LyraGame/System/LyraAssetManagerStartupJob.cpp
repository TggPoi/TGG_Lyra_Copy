// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAssetManagerStartupJob.h"

#include "LyraLogChannels.h"

TSharedPtr<FStreamableHandle> FLyraAssetManagerStartupJob::DoJob() const
{
	// 记录任务开始时间
	const double JobStartTime = FPlatformTime::Seconds();

	TSharedPtr<FStreamableHandle> Handle;
	UE_LOG(LogLyra, Display, TEXT("Startup job \"%s\" starting"), *JobName);

	//在这里真正执行任务
	JobFunc(*this, Handle);

	if (Handle.IsValid())
	{
		// 绑定资产的异步加载更新代理【如果需要使用这个句柄加载资产，可以使用UpdateSubstepProgressFromStreamable，更新资产的加载进度】
		/** 绑定一个委托函数，该函数会随着请求的更新而定期被调用，仅在加载过程中有效。此操作会覆盖任何已绑定的委托函数！*/
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate::CreateRaw(this, &FLyraAssetManagerStartupJob::UpdateSubstepProgressFromStreamable));

		// 等待执行完毕
		Handle->WaitUntilComplete(0.0f, false);

		// 取消代理
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate());
	}

	UE_LOG(LogLyra, Display, TEXT("Startup job \"%s\" took %.2f seconds to complete"), *JobName, FPlatformTime::Seconds() - JobStartTime);

	return Handle;
	
}
