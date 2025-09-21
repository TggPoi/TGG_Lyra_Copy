// Copyright Epic Games, Inc. All Rights Reserved.
// Finished. 已完成
#pragma once

#include "Engine/DataAsset.h"
#include "LyraExperienceDefinition.generated.h"

class UGameFeatureAction;
class ULyraPawnData;
class ULyraExperienceActionSet;

/**
 * Definition of an experience
 * 游戏体验的定义
 */
UCLASS(BlueprintType, Const)
class ULyraExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	ULyraExperienceDefinition();

	//~UObject interface
#if WITH_EDITOR
	/**
	 * Generic function to validate objects during changelist validations, etc.
	 * 用于在变更列表验证等过程中验证对象的通用函数。
	 *
	 * @param	Context	the context holding validation warnings/errors.
	 *			上下文：包含验证警告/错误的信息区域。
	 * @return Valid if this object has data validation rules set up for it and the data for this object is valid. Returns Invalid if it does not pass 
	 *         the rules. Returns NotValidated if no rules are set for this object.
	 *         如果此对象已设置数据验证规则且该对象的数据有效，则返回有效；否则返回无效。规则。如果此对象未设置任何规则，则返回“NotValidated”。
	 */	
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface


	
	//~UPrimaryDataAsset interface
#if WITH_EDITORONLY_DATA
	/** This scans the class for AssetBundles metadata on asset properties and initializes the AssetBundleData with InitializeAssetBundlesFromMetadata */
	/** 此函数会遍历该类中的资产属性，查找资产包的元数据，并使用“InitializeAssetBundlesFromMetadata”方法来初始化 AssetBundleData 对象 */ 
	virtual void UpdateAssetBundleData() override;
#endif
	//~End of UPrimaryDataAsset interface

public:
	// List of Game Feature Plugins this experience wants to have active
	// 此体验所需的已激活的游戏功能插件列表
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;

	/** The default pawn class to spawn for players */
	/** 用于生成玩家默认兵种的兵种类 */
	//@TODO: Make soft?
	//@待办事项：做成软引用？
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const ULyraPawnData> DefaultPawnData;

	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	// 当此体验被加载/激活/停用/卸载时要执行的操作列表
	/** Represents an action to be taken when a game feature is activated */
	/** 表示当某个游戏功能被激活时应采取的操作 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// List of additional action sets to compose into this experience
	// 用于构成此体验的附加操作集列表
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<TObjectPtr<ULyraExperienceActionSet>> ActionSets;
};