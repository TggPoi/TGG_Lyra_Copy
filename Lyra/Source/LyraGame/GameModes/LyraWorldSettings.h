// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Finished
#pragma once

#include "GameFramework/WorldSettings.h"
#include "LyraWorldSettings.generated.h"

#define UE_API LYRAGAME_API

class ULyraExperienceDefinition;

/**
 * The default world settings object, used primarily to set the default gameplay experience to use when playing on this map
 * 默认的世界设置对象，主要用于设定在该地图上游玩时所使用的默认游戏体验设置。
 */
UCLASS(MinimalAPI)
class ALyraWorldSettings : public AWorldSettings
{
	GENERATED_BODY()


public:

	UE_API ALyraWorldSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 *
	 * 该函数在“Map_Check”内部被调用，以便此角色能够检查自身是否存在任何潜在错误，并将这些错误信息通过“地图检查对话框”进行记录。
	 */
	UE_API virtual void CheckForErrors() override;
#endif

	

public:
	// Returns the default experience to use when a server opens this map if it is not overridden by the user-facing experience
	// 返回服务器在打开此地图时所使用的默认体验设置，若该设置未被用户界面中的体验所覆盖则使用此默认设置。
	UE_API FPrimaryAssetId GetDefaultGameplayExperience() const;
	
protected:
	
	// The default experience to use when a server opens this map if it is not overridden by the user-facing experience
	// 当服务器打开此地图时（若用户界面体验未对其进行覆盖），所采用的默认体验方式
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<ULyraExperienceDefinition> DefaultGameplayExperience;

	
public:
	
#if WITH_EDITORONLY_DATA
	// Is this level part of a front-end or other standalone experience?
	// When set, the net mode will be forced to Standalone when you hit Play in the editor
	// 这个级别是属于前端部分还是独立的独立体验的一部分呢？
	// 如果设置了此项，那么在编辑器中点击“播放”时，网络模式将强制切换为“独立模式”
	UPROPERTY(EditDefaultsOnly, Category=PIE)
	bool ForceStandaloneNetMode = false;
#endif
};





#undef UE_API