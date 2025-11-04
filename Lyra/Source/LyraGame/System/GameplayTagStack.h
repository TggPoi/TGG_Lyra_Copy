// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "GameplayTagStack.generated.h"

struct FGameplayTagStackContainer;
struct FNetDeltaSerializeInfo;

/**
 * Represents one stack of a gameplay tag (tag + count)
 * 表示一个游戏玩法标签的一组数据（标签 + 数量）
 * 主要用于处理游戏中的比分状态
 */
USTRUCT(BlueprintType)
struct FGameplayTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGameplayTagStack()
	{}

	FGameplayTagStack(FGameplayTag InTag, int32 InStackCount)
		: Tag(InTag)
		, StackCount(InStackCount)
	{
	}
	// 输出调试字段
	FString GetDebugString() const;
	
private:
	// 友元
	friend FGameplayTagStackContainer;
	
	// 使用的Tag
	// 例如死亡数可以使用lyra.gameplay.deaths标签，然后StackCount统计标签数量
	UPROPERTY()
	FGameplayTag Tag;

	// Tag的数量
	UPROPERTY()
	int32 StackCount = 0;

	
};

/** Container of gameplay tag stacks */
/** 游戏玩法标签堆栈的容器 */
USTRUCT(BlueprintType)
struct FGameplayTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FGameplayTagStackContainer()
	//	: Owner(nullptr)
	{
	}
public:
	
	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	// 向标签中添加指定数量的堆栈（若堆栈数量少于 1，则不执行任何操作）
	void AddStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	// 从标签中移除指定数量的堆栈（若“堆栈数量”小于 1，则不执行任何操作）
	void RemoveStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	// 返回指定标签的栈数量（若该标签不存在则返回 0）
	int32 GetStackCount(FGameplayTag Tag) const
	{
		return TagToCountMap.FindRef(Tag);
	}

	// Returns true if there is at least one stack of the specified tag
	// 如果存在指定标签的至少一个栈，则返回 true
	bool ContainsTag(FGameplayTag Tag) const
	{
		return TagToCountMap.Contains(Tag);
	}
	
	//~FFastArraySerializer contract 父类函数
	/**
	 * 在移除元素之前以及在元素自身收到通知之后调用。这些索引仅在本次函数调用中有效！*
	 * 注意：并非虚拟对象；通过模板代码调用，详见 FExampleItemEntry 。
	 * 
	 */
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	/**
	 * 在添加所有新元素之后以及元素自身收到通知之后触发。这些索引仅在本次函数调用中有效！*
	 * 注意：并非虚拟对象；通过模板代码调用，详见 FExampleItemEntry 。
	 * 
	 */
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	/**
	 * 在将所有现有元素的数据更新完毕并使这些元素自身收到通知之后调用。这些索引仅在本次函数调用中有效！*
	 * 注意：并非虚拟对象；通过模板代码调用，详见 FExampleItemEntry 。
	 * 
	 */
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameplayTagStack, FGameplayTagStackContainer>(Stacks, DeltaParms, *this);
	}
	
private:
	
	// Replicated list of gameplay tag stacks
	// 游戏玩法标签堆叠的复制列表
	UPROPERTY()
	TArray<FGameplayTagStack> Stacks;

	// Accelerated list of tag stacks for queries
	// 查询用标签栈的加速列表, 这个东西不需要网络同步,所以需要在同步前后进行手动修改
	// 缓存-用于快速查询
	TMap<FGameplayTag, int32> TagToCountMap;

	
};

/*
 * 针对结构体FGameplayTagStackContainer开启网络同步
 */
template<>
struct TStructOpsTypeTraits<FGameplayTagStackContainer> : public TStructOpsTypeTraitsBase2<FGameplayTagStackContainer>
{
	enum
	{
		// 开启Delta同步功能!
		WithNetDeltaSerializer = true,
	};
};

