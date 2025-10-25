// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Character/LyraCharacter.h"

#include "LyraCharacterWithAbilities.generated.h"

#define UE_API LYRAGAME_API


// ALyraCharacter typically gets the ability system component from the possessing player state
// This represents a character with a self-contained ability system component.
// ALyraCharacter通常会从拥有者的玩家状态中获取能力系统组件
// 这表示该角色拥有独立的能力系统组件。
UCLASS(MinimalAPI, Blueprintable)
class ALyraCharacterWithAbilities : public ALyraCharacter
{
	GENERATED_BODY()

public:
	/*
	 * 构造函数
	 * 1.创建GAS组件,设置网络同步模式
	 * 2.创建生命值组件
	 * 3.创建战斗组件
	 * 4.设置更新频率
	 */
	UE_API ALyraCharacterWithAbilities(const FObjectInitializer& ObjectInitializer);


};





#undef UE_API