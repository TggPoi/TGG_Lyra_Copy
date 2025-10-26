// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "GameFramework/PlayerStart.h"
#include "GameplayTagContainer.h"

#include "LyraPlayerStart.generated.h"

#define UE_API LYRAGAME_API
class AController;
class UObject;

enum class ELyraPlayerStartLocationOccupancy
{
	// 空闲
	Empty,
	// 部分占用
	Partial,
	// 完全占用
	Full
};

/**
 * ALyraPlayerStart
 * 
 * Base player starts that can be used by a lot of modes.
 * 基础角色可被多种模式所使用。
 */
UCLASS(MinimalAPI, Config = Game)
class ALyraPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	
	UE_API ALyraPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 获取目标位置的 是否可以放下Pawn的碰撞状态(大小是否合适).
	UE_API ELyraPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) const;

	/** Did this player start get claimed by a controller already? */
	// 是否被占用
	UE_API bool IsClaimed() const;

	
	/** If this PlayerStart was not claimed, claim it for ClaimingController */
	// 尝试占用
	UE_API bool TryClaim(AController* OccupyingController);

protected:

	/** Check if this PlayerStart is still claimed */
	/** 检查此玩家起点是否仍处于占用状态 */
	UE_API void CheckUnclaimed();
	
	/** The controller that claimed this PlayerStart */
	/** 承担此玩家起始点控制权的控制器 */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;

	
	/** Interval in which we'll check if this player start is not colliding with anyone anymore */
	/** 我们将在此间隔时间内检查此玩家的移动是否不再与任何人发生碰撞 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Start Claiming")
	float ExpirationCheckInterval = 1.f;

	/** Tags to identify this player start */
	/** 用于标识此玩家的标签开始 */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer StartPointTags;

	/** Handle to track expiration recurring timer */
	/** 用于跟踪定期过期定时器的标识符 */
	FTimerHandle ExpirationTimerHandle;
	
	
	
};





#undef UE_API