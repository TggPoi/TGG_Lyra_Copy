// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Not Finished 还需要更新GameplayCueManager的代码
#pragma once

#include "Engine/AssetManager.h"
#include "LyraAssetManagerStartupJob.h"
#include "Templates/SubclassOf.h"
#include "LyraAssetManager.generated.h"

#define UE_API LYRAGAME_API
class UPrimaryDataAsset;

class ULyraGameData;
class ULyraPawnData;

//一个约定的Bundles的命名
struct FLyraBundles
{
	static const FName Equipped;
};

/**
 * ULyraAssetManager
 *
 *	Game implementation of the asset manager that overrides functionality and stores game-specific types.
 *	It is expected that most games will want to override AssetManager as it provides a good place for game-specific loading logic.
 *	This class is used by setting 'AssetManagerClassName' in DefaultEngine.ini.
 *  资产管理器的实现，该实现会覆盖原有功能并存储游戏特定类型的数据。
 *  预计大多数游戏都会希望重写 AssetManager 类，因为它为游戏特定的加载逻辑提供了一个理想的位置。
 *  此类通过在 DefaultEngine.ini 中设置 'AssetManagerClassName' 来使用。
 * 
 */
UCLASS(MinimalAPI, Config = Game)
class ULyraAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	//构造函数 初始化PawnData
	UE_API ULyraAssetManager();
	
	// Returns the AssetManager singleton object.
	// 返回资产管理器的单例对象。
	static UE_API ULyraAssetManager& Get();

	// TSoftObjectPtr is templatized wrapper of the generic FSoftObjectPtr, it can be used in UProperties
	// TSoftObjectPtr 是通用 FSoftObjectPtr 的模板化封装类，可用于 UProperties 中。

	// Returns the asset referenced by a TSoftObjectPtr.  This will synchronously load the asset if it's not already loaded.
	// 返回由 TSoftObjectPtr 引用的资产。如果该资产尚未加载，则会同步加载该资产。
	template<typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// TSoftClassPtr is a templatized wrapper around FSoftObjectPtr that works like a TSubclassOf, it can be used in UProperties for blueprint subclasses
	// 返回由 TSoftClassPtr 指向的子类。如果该资产尚未加载，则会同步进行加载操作。

	// Returns the subclass referenced by a TSoftClassPtr.  This will synchronously load the asset if it's not already loaded.
	// 返回由 TSoftClassPtr 指向的子类。如果该资产尚未加载，则会同步进行加载操作。
	template<typename AssetType>
	static TSubclassOf<AssetType> GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// Logs all assets currently loaded and tracked by the asset manager.
	// 记录当前由资产管理器加载并跟踪的所有资产的信息。
	// 可以通过命令行调用，避免频繁通过断点查看
	static UE_API void DumpLoadedAssets();
	
	// 获取游戏数据
	UE_API const ULyraGameData& GetGameData();
	
	// 获取默认的玩家数据
	UE_API const ULyraPawnData* GetDefaultPawnData() const;


protected:

	// 获取或加载指定的游戏数据
	template <typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
	{
		// 如果已经缓存了直接获取
		if (TObjectPtr<UPrimaryDataAsset> const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}
		
		// Does a blocking load if needed
		// 如有需要则进行阻塞式加载
		return *CastChecked<const GameDataClass>(LoadGameDataOfClass(GameDataClass::StaticClass(),
			DataPath,
			GameDataClass::StaticClass()->GetFName()));
		
	}

	// 同步加载资产
	static UE_API UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);

	// 读取命令行参数,是否应当打印资产加载的日志
	// 打印日志也是有性能损耗的，所以这里可以配置是否打印日志
	static UE_API bool ShouldLogAssetLoads();

	// Thread safe way of adding a loaded asset to keep in memory.
	// 一种线程安全的添加已加载资源到内存中的方法。
	UE_API void AddLoadedAsset(const UObject* Asset);
	
	//~UAssetManager interface
	/** 开始初始加载，由“初始化对象引用--引擎初始化时调用的initObjectReference，详细看assetmanager加载流程”函数调用 */
	UE_API virtual void StartInitialLoading() override;
#if WITH_EDITOR
	/** Called right before PIE starts, will refresh asset directory and can be overriden to preload assets */
	/** （编辑器中点击开始按钮时调用）在 PIE 开始之前被调用，会刷新资源目录，并且可以在此处重写以实现预加载资源 */
	UE_API virtual void PreBeginPIE(bool bStartSimulate) override;
#endif

	/** 加载资产
	 * FPrimaryAssetType
	 * A primary asset type, represented as an FName internally and implicitly convertible back and forth
	 * This exists so the blueprint API can understand it's not a normal FName
	 *
	 * 一种主要的资产类型，内部以 FName 格式表示，并且可以自动进行双向转换
	 * 此设置的存在是为了让蓝图 API 能够明白这不是一个普通的 FName 类型
	 * 
	 */
	UE_API UPrimaryDataAsset* LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass,
		const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath,
		FPrimaryAssetType PrimaryAssetType);

	
protected:
	
	// Global game data asset to use.
	// 所需的全局游戏数据资源。
	// 这里是通过int配置.
	// 引擎初始化加载时使用，一开始默认不能为空
	UPROPERTY(Config)
	TSoftObjectPtr<ULyraGameData> LyraGameDataPath;

	// Loaded version of the game data
	// 已加载的游戏数据版本.
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UPrimaryDataAsset>> GameDataMap;
	
	// Pawn data used when spawning player pawns if there isn't one set on the player state.
	// 当玩家状态中未设置相关数据时，用于生成玩家兵卒的兵卒数据。
	// 这里是通过int配置.
	// 运行时动态加载，可以为空
	UPROPERTY(Config)
	TSoftObjectPtr<ULyraPawnData> DefaultPawnData;
	

private:
	// Flushes the StartupJobs array. Processes all startup work.
	// 清空“启动任务”数组。处理所有启动工作。
	UE_API void DoAllStartupJobs();
	
	// Sets up the ability system
	// 设置能力系统
	UE_API void InitializeGameplayCueManager();

	// Called periodically during loads, could be used to feed the status to a loading screen
	// 在加载过程中会定期调用此函数，可用于将状态信息传递给加载界面
	UE_API void UpdateInitialGameContentLoadPercent(float GameContentPercent);
	
	// The list of tasks to execute on startup. Used to track startup progress.
	// 启动时要执行的任务列表。用于跟踪启动过程的进度。
	TArray<FLyraAssetManagerStartupJob> StartupJobs;

private:
	
	// Assets loaded and tracked by the asset manager.
	// 资源已由资源管理器加载并进行跟踪。
	UPROPERTY()
	TSet<TObjectPtr<const UObject>> LoadedAssets;

	// Used for a scope lock when modifying the list of load assets.
	// 用于在修改加载资源列表时进行范围锁定。
	FCriticalSection LoadedAssetsCritical;
	
};

// 获取资产的模板实现
template<typename AssetType>
AssetType* ULyraAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;
	
	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		/**
		* 解除软指针的引用。
		* @返回值 若此对象已不存在或延迟指针为 null，则返回 nullptr；否则返回有效的 UObject 指针。
		* 
		 */
		LoadedAsset = AssetPointer.Get();
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPointer.ToString());
		}

		if (LoadedAsset && bKeepInMemory)
		{
			// Added to loaded asset list.
			// 已添加至已加载资源列表。
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
		
	}
	
	return LoadedAsset;
}

// 获取资产的类的模板实现
template<typename AssetType>
TSubclassOf<AssetType> ULyraAssetManager::GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedSubclass = AssetPointer.Get();

		if (!LoadedSubclass)
		{
			LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedSubclass, TEXT("Failed to load asset class [%s]"), *AssetPointer.ToString());
		}
		
		if (LoadedSubclass && bKeepInMemory)
		{
			// Added to loaded asset list.
			// 已添加至已加载资源列表。
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	
	}
		
	return LoadedSubclass;
}






#undef UE_API