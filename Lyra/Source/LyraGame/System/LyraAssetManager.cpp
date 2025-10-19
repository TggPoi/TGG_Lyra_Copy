// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAssetManager.h"
#include "LyraLogChannels.h"
#include "LyraGameplayTags.h"
#include "LyraGameData.h"
#include "AbilitySystemGlobals.h"
#include "Character/LyraPawnData.h"
#include "Misc/App.h"
#include "Stats/StatsMisc.h"
#include "Engine/Engine.h"
#include "AbilitySystem/LyraGameplayCueManager.h"
#include "Misc/ScopedSlowTask.h"
#include "System/LyraAssetManagerStartupJob.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraAssetManager)

//一个约定的Bundles的命名
const FName FLyraBundles::Equipped("Equipped");


//////////////////////////////////////////////////////////////////////
//通过命名行调用这个方法 打印加载的资产
static FAutoConsoleCommand CVarDumpLoadedAssets(
	TEXT("Lyra.DumpLoadedAssets"),
	TEXT("Shows all assets that were loaded via the asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateStatic(ULyraAssetManager::DumpLoadedAssets)
);


//////////////////////////////////////////////////////////////////////
// 添加一个任务到容器里面,这个任务就是传过来的JobFunc 并用Lambda包了一层
// 入参是函数,函数权重.
// 在Lambda包了一层函数的函数名作为字符串传递作为任务名
#define STARTUP_JOB_WEIGHTED(JobFunc, JobWeight) StartupJobs.Add(FLyraAssetManagerStartupJob(#JobFunc, [this](const FLyraAssetManagerStartupJob& StartupJob, TSharedPtr<FStreamableHandle>& LoadHandle){JobFunc;}, JobWeight))
#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc, 1.f)


//////////////////////////////////////////////////////////////////////
ULyraAssetManager::ULyraAssetManager()
{
	DefaultPawnData = nullptr;
}

ULyraAssetManager& ULyraAssetManager::Get()
{
	check(GEngine);

	if (ULyraAssetManager* Singleton = Cast<ULyraAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}
	
	UE_LOG(LogLyra, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to LyraAssetManager!"));

	// Fatal error above prevents this from being called.
	return *NewObject<ULyraAssetManager>();
}

void ULyraAssetManager::DumpLoadedAssets()
{
	UE_LOG(LogLyra, Log, TEXT("========== Start Dumping Loaded Assets =========="));

	for (const UObject* LoadedAsset : Get().LoadedAssets)
	{
		UE_LOG(LogLyra, Log, TEXT("  %s"), *GetNameSafe(LoadedAsset));
	}
	
	UE_LOG(LogLyra, Log, TEXT("... %d assets in loaded pool"), Get().LoadedAssets.Num());
	UE_LOG(LogLyra, Log, TEXT("========== Finish Dumping Loaded Assets =========="));
	
	
}

const ULyraGameData& ULyraAssetManager::GetGameData()
{
	// 错误的写法
	// ULyraGameData Temp;
	
	return GetOrLoadTypedGameData<ULyraGameData>(LyraGameDataPath);
}

const ULyraPawnData* ULyraAssetManager::GetDefaultPawnData() const
{
	return GetAsset(DefaultPawnData);
	//return nullptr;
}

UObject* ULyraAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		TUniquePtr<FScopeLogTime> LogTimePtr;
		//通过命名行确认是否需要打印该日志
		if (ShouldLogAssetLoads())
		{
			
			LogTimePtr = MakeUnique<FScopeLogTime>(*FString::Printf(TEXT("Synchronously loaded asset [%s]"),
			*AssetPath.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);

		}
		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}
		
		// Use LoadObject if asset manager isn't ready yet.
		// 若资产管理器尚未准备就绪，则使用 LoadObject 。
		return AssetPath.TryLoad();
	}
	return nullptr;
	
}

bool ULyraAssetManager::ShouldLogAssetLoads()
{
	static bool bLogAssetLoads = FParse::Param(FCommandLine::Get(), TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

void ULyraAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);
		
	}
}

void ULyraAssetManager::StartInitialLoading()
{

	// 专门用于 UE 启动阶段性能分析的工具，适合优化游戏启动时间
	SCOPED_BOOT_TIMING("ULyraAssetManager::StartInitialLoading");
	
	// This does all of the scanning, need to do this now even if loads are deferred
	// 这个会完成所有的扫描工作，即便加载操作被延迟，现在也需要执行此操作。
	Super::StartInitialLoading();
	
	// 申请一个任务 权重为1.f 去确认我们的GameplayCueManager是否正常
	// 这个任务函数
	STARTUP_JOB(InitializeGameplayCueManager());
	
	/* 这里的代码主要是给大家解释一下  这StartupJob任务宏的使用
	 *InitializeGameplayCueManager();

	//在FLyraAssetManagerStartupJob中提到的没有使用到的两个函数，按照下面的写法执行，才会使用到那两个函数
	auto Lambda = [This =this](const FLyraAssetManagerStartupJob& InMyJob, TSharedPtr<FStreamableHandle>& InMyHandle)
	{
		
		InMyJob.UpdateSubstepProgress(10.f);
		This->	InitializeGameplayCueManager();
		InMyJob.UpdateSubstepProgress(20.f);
		
	};
	
	FLyraAssetManagerStartupJob StartupJob(TEXT("TestJob"),Lambda,10.f);

	//但是这里只是一个任务，需要在DoAllStartupJobs 将每个任务都执行 UpdateInitialGameContentLoadPercent
	StartupJob.SubstepProgressDelegate.BindLambda(UpdateInitialGameContentLoadPercent()函数绑定，将加载内容提供给加载界面)

	StartupJobs.Add(StartupJob);
	*/
	
	{
		// Load base game data asset
		// 加载基础游戏数据文件
		STARTUP_JOB_WEIGHTED(GetGameData(), 25.f);
	}
	// Run all the queued up startup jobs
	// 执行所有已排队的启动任务
	DoAllStartupJobs();

	
}

#if WITH_EDITOR
void ULyraAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);

	{
		FScopedSlowTask SlowTask(0, NSLOCTEXT("LyraEditor", "BeginLoadingPIEData", "Loading PIE Data"));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);

		// 这里没有就崩了!!!
		const ULyraGameData& LocalGameDataCommon = GetGameData();
		
		// Intentionally after GetGameData to avoid counting GameData time in this timer
		// 有意安排在获取游戏数据之后进行，以避免将游戏数据的处理时间计入此计时器中
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("PreBeginPIE asset preloading complete"), nullptr);

		// You could add preloading of anything else needed for the experience we'll be using here
		// (e.g., by grabbing the default experience from the world settings + the experience override in developer settings)

		// 您可以预先加载我们在此处所使用体验所需的任何其他内容
		// （例如，通过从世界设置中获取默认体验，并结合开发人员设置中的体验覆盖内容）
		// 可以参考我们在EitodEngine下做的操作 希望实现在编辑器下的一些快速重写操作 比如强制网络模式为单机	
	}

	
}
#endif

UPrimaryDataAsset* ULyraAssetManager::LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass,
	const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	//用作性能分析
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object"), STAT_GameData, STATGROUP_LoadTime);

	//加载路径不能为空
	if (!DataClassPath.IsNull())
	{
		
#if WITH_EDITOR
		/**
		 * FScopedSlowTask一个表示工作量的划分单元，该单元被划分为若干部分。【使用方式在FScopedSlowTask类内部的注释中】
		 * 在每个函数的顶部使用一个范围块，以便向缓慢操作的用户提供准确的进度反馈。
		 * 
		 */
		FScopedSlowTask SlowTask(0,
			FText::Format(NSLOCTEXT("LyraEditor", "BeginLoadingGameDataTask", "Loading GameData {0}"), FText::FromName(DataClass->GetFName())));

		const bool bShowCancelButton = false;

		const bool bAllowInPIE = true;
		//在编辑器中生成对话框，方便调试
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
		
#endif
		
		UE_LOG(LogLyra, Log, TEXT("Loading GameData: %s ..."), *DataClassPath.ToString());

		// 自定计时打印日志的宏，轻量级快速打印日志
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("    ... GameData loaded!"), nullptr);

		// This can be called recursively in the editor because it is called on demand from PostLoad so force a sync load for primary asset and async load the rest in that case
		// 在编辑器中可以对该函数进行递归调用，因为它是根据需求从“PostLoad”阶段被调用的，所以在此情况下必须强制进行主资产的同步加载，并对其余部分进行异步加载。

		if (GIsEditor)
		{
			// 先加载主资产
			Asset = DataClassPath.LoadSynchronous();
			
			/** 加载指定类型的全部资源，适用于烘焙操作 */
			LoadPrimaryAssetsWithType(PrimaryAssetType);
		}
		else
		{
			TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);

			if (Handle.IsValid())
			{
				/**
				 * 直至所需资源加载完成才会停止。这会将所请求的资源推至优先级列表的首位，
				 * 但不会清除所有异步加载操作，通常会比调用 LoadObject 函数完成得更快。*
				 * @参数 Timeout：最大等待时间，若该值设为 0，则将一直等待
				 * @参数 StartStalledHandles：若为真，则会强制所有正在等待外部资源的句柄立即尝试加载
				 * 
				 */
				Handle->WaitUntilComplete(0.0f, false);

				// This should always work
				// 这种情况通常都会正常运行
				/** 返回所请求资产列表中的首个资产（若该资产已成功加载）。若该资产加载失败，则此操作将失败。 */
				Asset = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());

			}
			
		}

	}
	if (Asset)
	{
		//添加到缓存中 Key值是类类型 Value值是指针
		GameDataMap.Add(DataClass, Asset);
	}
	else
	{
		// It is not acceptable to fail to load any GameData asset. It will result in soft failures that are hard to diagnose.
		// 任何游戏数据资源的加载都不得失败。否则将会导致一些不易诊断的软性故障。
		UE_LOG(LogLyra, Fatal,
			TEXT("Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s."),
			*DataClassPath.ToString(), *PrimaryAssetType.ToString(), FApp::GetProjectName());
	}

	return Asset;
}

void ULyraAssetManager::DoAllStartupJobs()
{
	// 引擎启动计时分析
	SCOPED_BOOT_TIMING("ULyraAssetManager::DoAllStartupJobs");

	// 技术所有任务的开始时间
	const double AllStartupJobsStartTime = FPlatformTime::Seconds();

	/**
	 * 检查一下此可执行文件是否作为独立服务器进程启动，且不应加载仅客户端使用的数据。
	 * 可以通过在启动时使用“-server”参数来设置此选项为“真”，但在单进程“PlayInEditor”模式下则为“假”。
	 * 该功能不应用于游戏或网络用途，而应检查“NM_DedicatedServer”选项。
	 * 
	 */
	// 如果是运行在专属服务器上
	if (IsRunningDedicatedServer())
	{
		// No need for periodic progress updates, just run the jobs
		// 无需定期提供进展情况更新，直接运行这些任务即可
		
		for (const FLyraAssetManagerStartupJob& StartupJob : StartupJobs)
		{
			// 执行任务 这个句柄我们就没有接受 这点很有意思 这是个嵌入流程 因为我们写的都是简单任务 没有使用到句柄 所以在这里获取它没有啥作用
			// ReSharper disable once CppExpressionWithoutSideEffects
			StartupJob.DoJob();
		}
	}
	else
	{
		if (StartupJobs.Num() > 0)
		{
			// 总的进度
			float TotalJobValue = 0.0f;

			for (const FLyraAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				// 权重相加
				TotalJobValue += StartupJob.JobWeight;
			}
			
			// 累计推进的进度
			float AccumulatedJobValue = 0.0f;
			for (FLyraAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				// 拿到当前的任务总权重
				const float JobValue = StartupJob.JobWeight;

				// 绑定这个任务的进度更新 实际这玩意儿不会被调用 我们申请的都是通过宏设置的简单任务 一下就执行完了 完全没有去调用这个代理.
				// 只有像上面的案例一样 通过Lambda函数去申请任务的时候，手动调用InMyJob.UpdateSubstepProgress(10.f);函数 才会调用这个代理
				StartupJob.SubstepProgressDelegate.BindLambda([This = this, AccumulatedJobValue, JobValue, TotalJobValue](float NewProgress)
				{
					// 当前任务的进度
					const float SubstepAdjustment = FMath::Clamp(NewProgress, 0.0f, 1.0f) * JobValue;

					//  (之前已完成的任务进度权重+当前的任务已完成的权重)/总的任务权重
					const float OverallPercentWithSubstep = (AccumulatedJobValue + SubstepAdjustment) / TotalJobValue;

					This->UpdateInitialGameContentLoadPercent(OverallPercentWithSubstep);
				
					
				});

				// 执行任务 这里会阻塞等到直至完成
				// ReSharper disable once CppExpressionWithoutSideEffects
				StartupJob.DoJob();

				StartupJob.SubstepProgressDelegate.Unbind();

				AccumulatedJobValue += JobValue;

				// 更新界面
				UpdateInitialGameContentLoadPercent(AccumulatedJobValue / TotalJobValue);
			}
			
		}
		else
		{
			// 更新界面
			UpdateInitialGameContentLoadPercent(1.0f);
		}
	}

	// 清空任务容器
	StartupJobs.Empty();
	
	// 所有启动任务执行完毕
	UE_LOG(LogLyra, Display, TEXT("All startup jobs took %.2f seconds to complete"), FPlatformTime::Seconds() - AllStartupJobsStartTime);
	
}

void ULyraAssetManager::InitializeGameplayCueManager()
{
	// 专门用于 UE 启动阶段性能分析的工具，适合优化游戏启动时间
	SCOPED_BOOT_TIMING("ULyraAssetManager::InitializeGameplayCueManager");


	// ULyraGameplayCueManager* GCM = ULyraGameplayCueManager::Get();
	// check(GCM);
	// GCM->LoadAlwaysLoadedCues();
	//
	

	
}

void ULyraAssetManager::UpdateInitialGameContentLoadPercent(float GameContentPercent)
{
	// Could route this to the early startup loading screen
	// 可以将此内容转至早期启动加载界面
}
