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
//@待办事项：明确处理失败情况（进入“已完成但失败”的状态，而非通过检查来处理）

//@TODO: Do the action phases at the appropriate times instead of all at once
//@注意事项：应在恰当的时间执行各个行动阶段，而非一次性全部完成。

//@TODO: Support deactivating an experience and do the unloading actions
//@待办事项：支持停用体验，并执行卸载操作

//@TODO: Think about what deactivation/cleanup means for preloaded assets
//@待办事项：思考一下预加载资源的停用/清理工作意味着什么

//@TODO: Handle deactivating game features, right now we 'leak' them enabled
//@待办事项：处理游戏功能的停用问题，目前我们存在功能未停用而仍处于启用状态的情况。
// (for a client moving from experience to experience we actually want to diff the requirements and only unload some, not unload everything for them to just be immediately reloaded)
// （对于那些从一个项目转向另一个项目的客户而言，我们实际上希望对需求进行差异处理，只卸载一部分内容，而不是一次性全部卸载，以免导致他们需要重新加载所有内容）

//@TODO: Handle both built-in and URL-based plugins (search for colon?)
//@待办事项：处理内置插件和基于 URL 的插件（查找冒号？）

ULyraExperienceManagerComponent::ULyraExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	//开启默认网络同步
	SetIsReplicatedByDefault(true);
	
}

void ULyraExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
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

