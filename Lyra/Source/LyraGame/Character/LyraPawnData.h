// Copyright Epic Games, Inc. All Rights Reserved.
// 001 Not Finihed .还需要补充GAS能力集, 关系表,输入配置,相机模式
#pragma once

#include "Engine/DataAsset.h"

#include "LyraPawnData.generated.h"

#define UE_API LYRAGAME_API

class APawn;
class ULyraAbilitySet;
class ULyraAbilityTagRelationshipMapping;
class ULyraCameraMode;
class ULyraInputConfig;
class UObject;

/**
 * ULyraPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 *	一种不可变的数据资产，其中包含用于定义棋子的属性。
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Lyra Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class ULyraPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UE_API ULyraPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from ALyraPawn or ALyraCharacter).
	// 用于创建此兵种实例的类（通常应派生自 ALyraPawn 或 ALyraCharacter）。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	// 为该兵种赋予的能力组。
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
	//TArray<TObjectPtr<ULyraAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	// 此棋子执行行动时应采用何种能力标签的映射方式
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
	//TObjectPtr<ULyraAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	// 玩家控制的兵种所使用的输入配置，用于创建输入映射并绑定输入操作。
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Input")
	//TObjectPtr<ULyraInputConfig> InputConfig;

	// Default camera mode used by player controlled pawns.
	// 玩家控制的兵种所采用的默认摄像机模式。
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Camera")
	//TSubclassOf<ULyraCameraMode> DefaultCameraMode;

	
	
};






#undef UE_API
