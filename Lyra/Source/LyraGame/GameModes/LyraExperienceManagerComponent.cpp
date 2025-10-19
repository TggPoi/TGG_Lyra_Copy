// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraExperienceManagerComponent.h"

// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraExperienceManagerComponent.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "LyraExperienceDefinition.h"
#include "LyraExperienceActionSet.h"
#include "LyraExperienceManager.h"
#include "GameFeaturesSubsystem.h"
#include "System/LyraAssetManager.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystemSettings.h"
#include "TimerManager.h"
#include "Settings/LyraSettingsLocal.h"
#include "LyraLogChannels.h"

#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraExperienceManagerComponent)

//@TODO: Async load the experience definition itself
//@TODO: 异步加载Experience本身,我们现在是在设置Experience时直接进行加载的Tryload.

//@TODO: Handle failures explicitly (go into a 'completed but failed' state rather than check()-ing)
//@待办事项：明确处理失败情况（进入“已完成但失败”的状态，而非通过检查来处理--也就是说现在验证是否成功都是通过check断言验证状态，没有明确处理完成但是失败的情况）

//@TODO: Do the action phases at the appropriate times instead of all at once
//@注意事项：应在恰当的时间执行各个行动阶段，而非一次性全部完成。

//@TODO: Support deactivating an experience and do the unloading actions
//@待办事项：支持停用体验，并执行卸载操作

//@TODO: Think about what deactivation/cleanup means for preloaded assets
//@待办事项：思考一下预加载资源的停用/清理工作意味着什么
//在StartExperienceLoad中能够配置预加载的资产，如果存在这类资产，如何嵌入到当前的加载流程中？这个是后续拓展思考的内容

//@TODO: Handle deactivating game features, right now we 'leak' them enabled
//@待办事项：处理游戏功能的停用问题，目前我们存在功能未停用而仍处于启用状态的情况。
// (for a client moving from experience to experience we actually want to diff the requirements and only unload some, not unload everything for them to just be immediately reloaded)
// （对于那些从一个项目转向另一个项目的客户而言，我们实际上希望对需求进行差异处理，只卸载一部分内容，而不是一次性全部卸载，以免导致他们需要重新加载所有内容）

//@TODO: Handle both built-in and URL-based plugins (search for colon?)
//@待办事项：处理内置插件和基于 URL 的插件（查找冒号？）

//【lyra命令行变量】这是一个测试Experience随机延迟的命令行参数,它可以通过命令行输入读取随机延迟时间的最小值和最大值,并通过这个GetExperienceLoadDelayDuration()去获取到一个随机测试值.
namespace LyraConsoleVariables
{
	static float ExperienceLoadRandomDelayMin = 0.0f;
	
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayMin(
	TEXT("lyra.chaos.ExperienceDelayLoad.MinSecs"),
	ExperienceLoadRandomDelayMin,
	TEXT(
		"This value (in seconds) will be added as a delay of load completion of the experience (along with the random value lyra.chaos.ExperienceDelayLoad.RandomSecs)"),
	ECVF_Default);

	
	static float ExperienceLoadRandomDelayRange = 0.0f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayRange(
		TEXT("lyra.chaos.ExperienceDelayLoad.RandomSecs"), //通过在控制台输入这个命令，配置随机数变量 ExperienceLoadRandomDelayRange
		ExperienceLoadRandomDelayRange,
		TEXT(
			"A random amount of time between 0 and this value (in seconds) will be added as a delay of load completion of the experience (along with the fixed value lyra.chaos.ExperienceDelayLoad.MinSecs)"),
		ECVF_Default);
		
	float GetExperienceLoadDelayDuration()
	{
		return FMath::Max(0.0f, ExperienceLoadRandomDelayMin + FMath::FRand() * ExperienceLoadRandomDelayRange);
	}
	
}

ULyraExperienceManagerComponent::ULyraExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	//开启默认网络同步
	SetIsReplicatedByDefault(true);
	
}

void ULyraExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// deactivate any features this experience loaded
	// 恢复取消加载的任何功能设置
	//@TODO: This should be handled FILO as well
	//@待办事项：此事也应按照后进先出的原则来处理。

	
	//关闭游戏特效插件
	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		//通过我们写的引擎子系统来确认这个插件确实已经所有依赖释放完毕 (引用计数都已经归0时才可以关闭) ,最终释放这个插件.
		if (ULyraExperienceManager::RequestToDeactivatePlugin(PluginURL))
		{
			UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
		}
	}

	//@TODO: Ensure proper handling of a partially-loaded state too
	//@待办事项：还需确保对部分加载状态进行妥善处理

	if (LoadState == ELyraExperienceLoadState::Loaded)
	{
		LoadState = ELyraExperienceLoadState::Deactivating;

		// Make sure we won't complete the transition prematurely if someone registers as a pauser but fires immediately
		// 确保即便有人注册为暂停者但随即立即执行操作，我们也不会过早完成转换过程。（使用期望值和观察值两个变量限制）
		NumExpectedPausers = INDEX_NONE;
		NumObservedPausers = 0;

		// Deactivate and unload the actions
		// 反激活并卸载这些操作
		
		//在上下文中绑定Action结束时要进行的操作
		FGameFeatureDeactivatingContext Context(
			TEXT(""), [this](FStringView) { this->OnActionDeactivationCompleted(); });
		
		//指定上下文执行的世界
		const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
		if (ExistingWorldContext)
		{
			Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
		}

		//取消Action的Lambda
		auto DeactivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
		{
			for (UGameFeatureAction* Action : ActionList)
			{
				if (Action)
				{
					Action->OnGameFeatureDeactivating(Context);
					Action->OnGameFeatureUnregistering();
				}
			}
		};

		//取消Experience中的Actions
		DeactivateListOfActions(CurrentExperience->Actions);

		//取消Experience中的ActionSets的Action
		for (const TObjectPtr<ULyraExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
		{
			if (ActionSet != nullptr)
			{
				DeactivateListOfActions(ActionSet->Actions);
			}
		}
		
		NumExpectedPausers = Context.GetNumPausers();
		
		//现在还不支持异步去取消这些操作,所以此处应该是0
		//所以在执行 DeactivateListOfActions 的lambda函数时就已经取消Actions了，而不是等到Context上下文结束
		if (NumExpectedPausers > 0)
		{
			UE_LOG(LogLyraExperience, Error,
				   TEXT("Actions that have asynchronous deactivation aren't fully supported yet in Lyra experiences"));
		}

		// 所有的Action都已观测到取消,观测到的值和期望值是相等的，这里时同步的,所以会执行
		if (NumExpectedPausers == NumObservedPausers)
		{
			OnAllActionsDeactivated();
		}
		
	}
	
	
}

void ULyraExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentExperience);
}

bool ULyraExperienceManagerComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (LoadState != ELyraExperienceLoadState::Loaded)
	{
		OutReason = TEXT("Experience still loading");
		return true;
	}
	else
	{
		return false;
	}
}

void ULyraExperienceManagerComponent::SetCurrentExperience(FPrimaryAssetId ExperienceId)
{

	UAssetManager& AssetManager = UAssetManager::Get();

	/** 获取指定主资产类型及名称的 FSoftObjectPath，若未找到则返回无效值 */
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);

	//尝试加载该资源时，这将调用“LoadObject”函数，而该函数的执行可能会非常缓慢。
	TSubclassOf<ULyraExperienceDefinition> AssetClass = Cast<UClass>(AssetPath.TryLoad());

	//必须要拿到!
	check(AssetClass);

	//获取一个类的默认对象。
	//在大多数情况下，类的默认对象不应被修改。因此，此方法返回的是一个不可变的指针。如果您需要修改默认对象，请使用 GetMutableDefault 替代。
	const ULyraExperienceDefinition* Experience = GetDefault<ULyraExperienceDefinition>(AssetClass);

	check(Experience != nullptr);
	//确保是第一次加载，否则表示加载流程错误了，游戏刚开始应该是空的，还没有设置
	check(CurrentExperience == nullptr);

	//这里由服务器属性同步到到客户端进行加载
	CurrentExperience = Experience;
	
	//开始加载
	StartExperienceLoad();
	
}

void ULyraExperienceManagerComponent::OnRep_CurrentExperience()
{
	//从服务器同步过来,开启加载
	StartExperienceLoad();
}

void ULyraExperienceManagerComponent::StartExperienceLoad()
{
	//必须是正确的初始化状态,不能是空指针,也不能多次加载,要不然就是流程错误
	check(CurrentExperience != nullptr);
	check(LoadState == ELyraExperienceLoadState::Unloaded);

	UE_LOG(LogLyraExperience, Log, TEXT("EXPERIENCE: StartExperienceLoad(CurrentExperience = %s, %s)"),
		   *CurrentExperience->GetPrimaryAssetId().ToString(),
		   *GetClientServerContextString(this));
	
	//切换到正在加载的状态
	LoadState = ELyraExperienceLoadState::Loading;

	UAssetManager& AssetManager = UAssetManager::Get();

	//需要通过Bundle进行加载处理的资产
	//---【当前资产存在状态，需要手动处理，例如有些资产是服务器加载，有些是客户端加载】
	TSet<FPrimaryAssetId> BundleAssetList;

	//只需要加载的资产
	TSet<FSoftObjectPath> RawAssetList;

	//添加我们正在使用的Experience的资产
	BundleAssetList.Add(CurrentExperience->GetPrimaryAssetId());

	//添加这个Experience所携带的ActionsSets.注意不是Actions
	for (const TObjectPtr<ULyraExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
		}
	}

	// Load assets associated with the experience
	// 加载与该体验相关的资源

	//需要传递的这些Bundles
	TArray<FName> BundlesToLoad;

	// 这里添加了资产管理类里面的一个全局变量,它是一个Bundle规则.
	// @TODO: 目前没有定义，但是应该是使用 BundlesToLoad.Add(FLyraBundles::Equipped);
	BundlesToLoad.Add(TEXT("Equipped"));


	//@TODO: Centralize this client/server stuff into the LyraAssetManager
	//@待办事项：将此客户端/服务器相关的内容集中到“LyraAssetManager”中。

	//获取当前的网络模式,是通过GameState的方式来获取
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	//如果是编辑器就都要加载
	const bool bLoadClient = GIsEditor || (OwnerNetMode != NM_DedicatedServer);
	const bool bLoadServer = GIsEditor || (OwnerNetMode != NM_Client);

	if (bLoadClient)
	{
		/** State/Bundle to always load on clients */
		/** 用于始终在客户端加载的配置/数据包 */
		//Client
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	
	if (bLoadServer)
	{
		/** State/Bundle to always load on dedicated server */
		/** 用于在专用服务器上始终加载的配置/数据包 */
		//Server
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	// 一个用于同步或异步加载的句柄。只要该句柄处于激活状态，加载的资源就会保留在内存中。
	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;

	if (BundleAssetList.Num() > 0)
	{
		// Changes the bundle state of a set of loaded primary assets.
		// wait on the returned streamable request or poll as needed.
		// If there is no work to do, returned handle will be null and delegate will get called.
		// Prefer the overloads with Param structs for new code.

		// 更改一组已加载的主资源的捆绑状态。
		// 等待返回的可流式请求完成，或根据需要进行轮询。
		// 如果没有需要执行的工作，返回的句柄将为 null，并会调用 delegate 方法。
		// 建议使用带有 Param 结构体的重载版本来编写新代码。

		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList.Array(),
																		  BundlesToLoad,
																		  {},
																		  false,
																		  FStreamableDelegate(),
																		  FStreamableManager::AsyncLoadHighPriority);
		
	}

	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;
	
	if (RawAssetList.Num() > 0)
	{
		// Load non primary assets with the primary streamable manager.
		// This will not auto release the handle, release it if needed.
		// Prefer the overloads with Param structs for new code.
		// 使用主流式管理器加载非主要资源。
		// 该操作不会自动释放句柄，如有需要请手动释放。
		// 对于新代码，建议使用带有参数结构体的重载版本。
		RawLoadHandle = AssetManager.LoadAssetList(RawAssetList.Array(),
												   FStreamableDelegate(),
												   FStreamableManager::AsyncLoadHighPriority,
												   TEXT("StartExperienceLoad()"));
	}

	// If both async loads are running, combine them
	// 如果两个异步加载操作都在进行中，则将它们合并起来

	TSharedPtr<FStreamableHandle> Handle = nullptr;
	
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		// Creates a combined handle, which will wait for other handles to complete before completing. The child handles will be held as hard references as long as this handle is active.
		// 创建一个组合型句柄，该句柄会在其他句柄完成之前一直等待其完成。只要此句柄处于活动状态，相关子句柄就会保持为强引用关系。
		Handle = AssetManager.GetStreamableManager().CreateCombinedHandle({BundleLoadHandle, RawLoadHandle});
		
	}
	else
	{
		//选择其中一个句柄作为可用的句柄
		Handle = BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
	}

	//创建一个流式加载的代理，加载完成执行的代理
	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(
		this, &ThisClass::OnExperienceLoadComplete);
	
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		// Assets were already loaded, call the delegate now
		// 资源已加载完成，现在调用委托函数即可
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		// Bind delegate that is called when load completes, only works if loading is in progress. This will overwrite any already bound delegate!
		// 绑定在加载完成时执行的委托函数，仅在加载过程中有效。此操作会覆盖任何已绑定的委托函数！
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);

		// Bind delegate that is called if handle is canceled, only works if loading is in progress. This will overwrite any already bound delegate!
		// 如果处理操作被取消，则会调用此绑定的委托。仅在加载操作进行时才有效。此操作会覆盖任何已绑定的委托！
		//---Handle->BindCancelDelegate(OnAssetsLoadedDelegate);这种写法也行（秀操作），下面这种写法能在执行代理前，先执行一些逻辑操作，比如取消加载，或者执行一些其他操作
		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
		{
			OnAssetsLoadedDelegate.ExecuteIfBound();
			
		}));
	}
	
	// This set of assets gets preloaded, but we don't block the start of the experience based on it
	// 预先加载资产，这里不需要句柄，这些资源会预先加载，但我们不会因此而阻止体验的开始。（例如有一些资产不是一开始必须要的，可以慢慢加载，不需要堵塞）
	TSet<FPrimaryAssetId> PreloadAssetList;
	//@TODO: Determine assets to preload (but not blocking-ly)
	//@待办事项：确定需要预先加载的资源（但不进行阻塞式加载）
	if (PreloadAssetList.Num() > 0)
	{
		AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(), BundlesToLoad, {});
	}
	
	
}

void ULyraExperienceManagerComponent::OnExperienceLoadComplete()
{
	check(LoadState == ELyraExperienceLoadState::Loading);

	check(CurrentExperience != nullptr);

	UE_LOG(LogLyraExperience, Log, TEXT("EXPERIENCE: OnExperienceLoadComplete(CurrentExperience = %s, %s)"),
		   *CurrentExperience->GetPrimaryAssetId().ToString(),
		   *GetClientServerContextString(this));

	// find the URLs for our GameFeaturePlugins - filtering out dupes and ones that don't have a valid mapping
	// 找出我们游戏功能插件的网址——剔除重复项以及那些没有有效映射关系的网址
	GameFeaturePluginURLs.Reset();

	// 搜集要使用的所有GameFeature插件
	// Context作为一个上下文变量,可能会使用到,但目前没有
	auto CollectGameFeaturePluginURLs = [This=this](const UPrimaryDataAsset* Context,
													const TArray<FString>& FeaturePluginList)
	{
		for (const FString& PluginName : FeaturePluginList)
		{
			FString PluginURL;
			//需要对这些插件名字和URL进行验证,因为有可能写错了插件名导致找不到
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, /*out*/ PluginURL))
			{
				This->GameFeaturePluginURLs.AddUnique(PluginURL);
			}
			else
			{
				ensureMsgf(
					false,
					TEXT(
						"OnExperienceLoadComplete failed to find plugin URL from PluginName %s for experience %s - fix data, ignoring for this run"
					), *PluginName, *Context->GetPrimaryAssetId().ToString());
			}
		}
		// 		// Add in our extra plugin
		// 		if (!CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent.IsEmpty())
		// 		{
		// 			FString PluginURL;
		// 			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent, PluginURL))
		// 			{
		// 				GameFeaturePluginURLs.AddUnique(PluginURL);
		// 			}
		// 		}

		
	};


	CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeaturesToEnable);

	//把ActionsSets中的每一个ActionSet对应的所有GameFeatures插件都填进去
	for (const TObjectPtr<ULyraExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnable);
		}
	}

	// Load and activate the features
	// 加载并启用各项功能

	//记录所有需要开启的游戏特性插件总数
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();

	if (NumGameFeaturePluginsLoading > 0)
	{
		LoadState = ELyraExperienceLoadState::LoadingGameFeatures;

		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			//增加使用计数
			ULyraExperienceManager::NotifyOfPluginActivation(PluginURL);

			//激活该插件.在该插件激活完毕后触发是否Experience完全加载的判定
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(
				PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(
					this, &ThisClass::OnGameFeaturePluginLoadComplete));
			
		}
		
	}
	else
	{
		//如果没有,直接直接调用Experience充分加载这个函数
		OnExperienceFullLoadCompleted();
	}

}

void ULyraExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	// decrement the number of plugins that are loading
	// 减少正在加载的插件数量
	NumGameFeaturePluginsLoading--;

	if (NumGameFeaturePluginsLoading == 0)
	{
		OnExperienceFullLoadCompleted();
	}
}

void ULyraExperienceManagerComponent::OnExperienceFullLoadCompleted()
{
	//到这一步一定是出于loading状态，或者正在加载Gamefeature的状态
	check(LoadState != ELyraExperienceLoadState::Loaded);
	
	// Insert a random delay for testing (if configured)
	// （如果已配置）插入一段随机延迟以进行测试
	// 在需要加载大量数据的情况下，随机插入一段延迟开始递归调用OnExperienceFullLoadCompleted
	if (LoadState  != ELyraExperienceLoadState::LoadingChaosTestingDelay)
	{
		const float DelaySecs =  LyraConsoleVariables::GetExperienceLoadDelayDuration();
		if (DelaySecs > 0.0f)
		{
			FTimerHandle DummyHandle;
			
			LoadState = ELyraExperienceLoadState::LoadingChaosTestingDelay;
			
			GetWorld()->GetTimerManager().SetTimer(DummyHandle, this, &ThisClass::OnExperienceFullLoadCompleted,
									   DelaySecs, /*bLooping=*/ false);
			
			return;
		}
	}

	//切换状态到执行Actions
	LoadState = ELyraExperienceLoadState::ExecutingActions;
	
	// Execute the actions
	// 执行这些操作
	
	FGameFeatureActivatingContext Context;

	// Only apply to our specific world context if set
	// 只有在设置的情况下才适用于我们特定的环境背景
	
	/*
	* FWorldContext 是用于在引擎层面处理 UWorld 的一个上下文环境。当引擎启动和销毁世界时，我们需要一种方法来明确区分哪些世界属于哪个世界。
	* WorldContext 可以被视为一条轨道。默认情况下，我们有一个轨道用于加载和卸载关卡。添加第二个上下文就是添加第二条轨道；为世界提供另一个存在的进展轨道。
	* 对于游戏引擎，直到我们决定支持多个同时存在的世界之前，将只有一个 WorldContext。对于编辑器引擎，可能会有一个用于编辑器世界和一个用于 PIE 世界的 WorldContext。
	* FWorldContext 提供了管理“当前的 PIE UWorld*”的方法，以及与连接/前往新世界相关的状态。
	* FWorldContext 应该保持在 UEngine 类内部。外部代码不应保留指针或直接管理 FWorldContext。外部代码仍然可以处理 UWorld*，并将 UWorld* 传递给引擎级别的函数。引擎代码可以根据给定的 UWorld* 查找相关的上下文。
	* 为了方便起见，FWorldContext 可以维护外部对 UWorld* 的指针。例如，PIE 可以将 UWorld* UEditorEngine:：PlayWorld 与 PIE 世界上下文关联起来。如果 PIE UWorld 发生变化，UEditorEngine::PlayWorld 指针将自动更新。这是通过调用 AddRef() 和 SetCurrentWorld() 来实现的。
	*/
	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());

	if (ExistingWorldContext)
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	//执行Action操作的Lambda,需要提供一个世界的上下文.
	//ActionList：需要激活的Action
	auto ActivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action != nullptr)
			{
				//@TODO: The fact that these don't take a world are potentially problematic in client-server PIE
				//@待办事项：这些机制不涉及整个世界这一事实，在客户端-服务器的 PIE 中可能会存在问题。
				//例如类似 抽卡机制 的组件，这类组件需要服务端做计算，但是又不希望客户端的Actor拥有这些组件，这种情况下可能有bug

				// The current behavior matches systems like gameplay tags where loading and registering apply to the entire process,
				// 目前的这种行为与诸如游戏玩法标签这样的系统相匹配，在这些系统中，加载和注册操作适用于整个过程。

								
				// but actually applying the results to actors is restricted to a specific world
				// 但实际上，将这些结果应用于演员这一环节却受到特定环境的限制
				
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}

	};
	
	ActivateListOfActions(CurrentExperience->Actions);

	//执行ActionSet的操作
	for (const TObjectPtr<ULyraExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			ActivateListOfActions(ActionSet->Actions);
		}
	}

	//到这里加载完成了.
	LoadState = ELyraExperienceLoadState::Loaded;

	// 呼叫执行各个级别的代理 这里通过优先级的控制 使得代理事件之间可以进行时序的区分
	OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_HighPriority.Clear();

	OnExperienceLoaded.Broadcast(CurrentExperience);
	OnExperienceLoaded.Clear();

	OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_LowPriority.Clear();


	// Apply any necessary scalability settings
	// 应用任何必要的扩展性设置
	
// #if !UE_SERVER 还不需要实现，在客户端进行图形，音频设置
// 	ULyraSettingsLocal::Get()->OnExperienceLoaded();
// #endif
	
}

void ULyraExperienceManagerComponent::OnActionDeactivationCompleted()
{
	//对于正在退出的Action进行计数
	check(IsInGameThread());
	++NumObservedPausers;
	// 所有的Action都已观测到取消,这里应该是异步的执行,但是 EndPlay 代码给的日志,提示目前还不支持异步的操作
	if (NumObservedPausers == NumExpectedPausers)
	{
		OnAllActionsDeactivated();
	}
	
}

void ULyraExperienceManagerComponent::OnAllActionsDeactivated()
{
	//@TODO: We actually only deactivated and didn't fully unload...
	//@待办事项：实际上我们只是暂时停用了，并未完全卸载……

	LoadState = ELyraExperienceLoadState::Unloaded;
	CurrentExperience = nullptr;
	
	//@TODO:	GEngine->ForceGarbageCollection(true);
	//@待办事项：GEngine->强制执行垃圾回收（true）；
	
}

void ULyraExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority(
	FOnLyraExperienceLoaded::FDelegate&& Delegate)
{
	//如果是已经加载了就直接执行代理即可
	//如果尚未加载完成,存到对应优先级级别的容器中,等待加载完成后统一按优先级顺序调用
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_HighPriority.Add(MoveTemp(Delegate));
	}
}

void ULyraExperienceManagerComponent::CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate&& Delegate)
{
	//如果是已经加载了就直接执行代理即可
	//如果尚未加载完成,存到对应优先级级别的容器中,等待加载完成后统一按优先级顺序调用
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(Delegate));
	}
}

void ULyraExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority(
	FOnLyraExperienceLoaded::FDelegate&& Delegate)
{
	//如果是已经加载了就直接执行代理即可
	//如果尚未加载完成,存到对应优先级级别的容器中,等待加载完成后统一按优先级顺序调用
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_LowPriority.Add(MoveTemp(Delegate));
	}
	
}

const ULyraExperienceDefinition* ULyraExperienceManagerComponent::GetCurrentExperienceChecked() const
{
	//这个函数必须在加载完成后调用,且必须加载成功.
	check(LoadState == ELyraExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);

	return CurrentExperience;
	
}

bool ULyraExperienceManagerComponent::IsExperienceLoaded() const
{
	//判断加载状态以及体验必须存在.
	return (LoadState == ELyraExperienceLoadState::Loaded) && (CurrentExperience != nullptr);
}

