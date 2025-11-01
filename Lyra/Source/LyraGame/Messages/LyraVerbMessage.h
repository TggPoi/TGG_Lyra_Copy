// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "LyraVerbMessage.generated.h"

// Represents a generic message of the form Instigator Verb Target (in Context, with Magnitude)
// 表示一种通用的消息形式，即“起因 + 动词 + 目标（在特定情境下，包含强度）* 本项目所使用的基础能力系统组件类”
USTRUCT(BlueprintType)
struct FLyraVerbMessage
{
	GENERATED_BODY()

	// 动词 表示行为的Tag
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTag Verb;

	// 主语 发起者的指针
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Instigator = nullptr;
		
	// 宾语 接受者的指针
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Target = nullptr;

	// 定语 发起者的状态Tag
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer InstigatorTags;

	// 定语 接受者的状态Tag
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer TargetTags;

	// 状语 上下文的Tag
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer ContextTags;

	// 量级 表示具体的数量
	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	double Magnitude = 1.0;

	// Returns a debug string representation of this message
	// 返回此消息的调试字符串表示形式
	LYRAGAME_API FString ToString() const;

	
	
};