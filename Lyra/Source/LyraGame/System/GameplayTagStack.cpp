// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTagStack.h"

#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayTagStack)

//////////////////////////////////////////////////////////////////////
// FGameplayTagStack

FString FGameplayTagStack::GetDebugString() const
{
	return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), StackCount);
}

void FGameplayTagStackContainer::AddStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddStack"), ELogVerbosity::Warning);
		return;
	}
	
	if (StackCount > 0)
	{
		for (FGameplayTagStack& Stack : Stacks)
		{
			if (Stack.Tag == Tag)
			{
				const int32 NewCount = Stack.StackCount + StackCount;
				Stack.StackCount = NewCount;
				TagToCountMap[Tag] = NewCount;
				
				// 修改了元素 必须标记为脏!
				MarkItemDirty(Stack);
				return;
			}

		}

		FGameplayTagStack& NewStack = Stacks.Emplace_GetRef(Tag, StackCount);
		// 新增了元素 必须标记为脏
		MarkItemDirty(NewStack);
		
		// 方便快捷查询
		TagToCountMap.Add(Tag, StackCount);
	}
}

void FGameplayTagStackContainer::RemoveStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStack"), ELogVerbosity::Warning);
		return;
	}
	
	//@TODO: Should we error if you try to remove a stack that doesn't exist or has a smaller count?
	//@待办事项：如果尝试删除一个并不存在或数量更少的栈，我们是否应该报错？

	if (StackCount > 0)
	{
		for (auto It = Stacks.CreateIterator(); It; ++It)
		{
			FGameplayTagStack& Stack = *It;
			if (Stack.Tag == Tag)
			{

				if (Stack.StackCount <= StackCount)
				{
					It.RemoveCurrent();
					TagToCountMap.Remove(Tag);
					// 移除了元素 必须标记整个数组脏了!
					MarkArrayDirty();
				}
				else
				{
					const int32 NewCount = Stack.StackCount - StackCount;
					Stack.StackCount = NewCount;
					TagToCountMap[Tag] = NewCount;
					
					// 修改了元素 必须标记它脏了!
					MarkItemDirty(Stack);
				}
				return;
			}
		}
	}

}

/*
*Unreal Engine 5中的TArrayView<int>是一种轻量级的、非拥有式的视图，用于查看一段连续的整数数据。它无需复制或拥有内存，就能安全、高效地访问现有的数组。

关于TArrayView<int>的要点：
• 非拥有式：不管理内存，仅引用已有的数据。
• 轻量级：只存储一个指针和一个大小，不进行内存分配。
• 灵活性：可以从TArray<int>、原始C风格数组或指针创建。
• 不可变视图选项：使用TArrayView<const int>实现只读访问。
 */
void FGameplayTagStackContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// 受到同步删除前执行
	for (int32 Index : RemovedIndices)
	{
		const FGameplayTag Tag = Stacks[Index].Tag;
		// 移除Map中的快速查询
		TagToCountMap.Remove(Tag);
	}
	
	
}

void FGameplayTagStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// 收到同步添加后执行
	for (int32 Index : AddedIndices)
	{
		const FGameplayTagStack& Stack = Stacks[Index];
		// 增加Map中的快速查询
		TagToCountMap.Add(Stack.Tag, Stack.StackCount);
	}	
}

void FGameplayTagStackContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	// 受到同步添加改变后执行
	for (int32 Index : ChangedIndices)
	{
		const FGameplayTagStack& Stack = Stacks[Index];
		TagToCountMap[Stack.Tag] = Stack.StackCount;
	}
}

//////////////////////////////////////////////////////////////////////
// FGameplayTagStackContainer
