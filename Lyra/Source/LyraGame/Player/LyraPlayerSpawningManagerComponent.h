// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "Components/GameStateComponent.h"

#include "LyraPlayerSpawningManagerComponent.generated.h"

#define UE_API LYRAGAME_API

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class ALyraPlayerStart;
class AActor;



/**
 * @class ULyraPlayerSpawningManagerComponent
 *
 * 用来辅助GameMode管理玩家的生成
 * 
 */
UCLASS(MinimalAPI)
class ULyraPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UE_API ULyraPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	/** UActorComponent */
	UE_API virtual void InitializeComponent() override;
	UE_API virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** ~UActorComponent */
	
protected:
	
	// Utility
	// 找到第一个随机的未被占用的玩家出生点
	UE_API APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<ALyraPlayerStart*>& FoundStartPoints) const;
	
	// 自定义的玩家完成重启事件
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) { }

	// 用于蓝图拓展的自定义玩家完成重启事件
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	UE_API void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);
	
	// 自定义的筛选规则,需要自行拓展
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<ALyraPlayerStart*>& PlayerStarts) { return nullptr; }


	
private:

	/** We proxy these calls from ALyraGameMode, to this component so that each experience can more easily customize the respawn system they want. */
	/** 我们从 AlyraGameMode 中对这些调用进行代理处理，使其传递至本组件，这样每个体验就能更方便地根据自身需求定制重生系统。*/
	UE_API AActor* ChoosePlayerStart(AController* Player);
	UE_API bool ControllerCanRestart(AController* Player);
	UE_API void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class ALyraGameMode;
	/** ~ALyraGameMode */




	
private:

	
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALyraPlayerStart>> CachedPlayerStarts;
private:
	// 处理新增关卡的玩家出生点逻辑
	UE_API void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	// 处理动态生成的玩家出生点
	UE_API void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	// PIE模型下寻找最近出生点
	UE_API APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
	
};




#undef UE_API