// Copyright Epic Games, Inc. All Rights Reserved.

#include "AsyncAction_ExperienceReady.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ExperienceReady)

UAsyncAction_ExperienceReady::UAsyncAction_ExperienceReady(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UAsyncAction_ExperienceReady* UAsyncAction_ExperienceReady::WaitForExperienceReady(UObject* InWorldContextObject)
{
	UAsyncAction_ExperienceReady* Action = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		Action = NewObject<UAsyncAction_ExperienceReady>();
		Action->WorldPtr = World;
		
		// 这里是注册到了世界上，避免被GC回收，让这个对象的生命周期和世界（游戏实例）保持一致
		Action->RegisterWithGameInstance(World);
	}
	return Action;
}

void UAsyncAction_ExperienceReady::Activate()
{
	if (UWorld* World = WorldPtr.Get())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			// 有GameState 直接通过GameState实现
			Step2_ListenToExperienceLoading(GameState);
		}
		else
		{
			// GameState还没生成,绑定在GameState生成事件上
			World->GameStateSetEvent.AddUObject(this, &ThisClass::Step1_HandleGameStateSet);
		}
		
	}
	else
	{
		// No world so we'll never finish naturally
		// 没有世界,无法执行,直接销毁
		SetReadyToDestroy();
	}
	

	
}

void UAsyncAction_ExperienceReady::Step1_HandleGameStateSet(AGameStateBase* GameState)
{
	if (UWorld* World = WorldPtr.Get())
	{	// 清楚之前来自我这个对象的绑定.
		World->GameStateSetEvent.RemoveAll(this);
	}

	Step2_ListenToExperienceLoading(GameState);
	
}

void UAsyncAction_ExperienceReady::Step2_ListenToExperienceLoading(AGameStateBase* GameState)
{
	check(GameState);
	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	// 是否已经加载了 如果加载了就直接调用 但是直接调用可能会有主逻辑问题 那么最好手动做一帧的延迟
	// 没有加载就绑定在代理上即可
	if (ExperienceComponent->IsExperienceLoaded())
	{
		UWorld* World = GameState->GetWorld();
		check(World);

		// The experience happened to be already loaded, but still delay a frame to
		// make sure people don't write stuff that relies on this always being true

		// 这段代码中的逻辑已经得到了执行，但为了确保不会出现因某些操作依赖于该逻辑始终为真而导致的问题，还是再延迟了一帧。

		
		//@TODO: Consider not delaying for dynamically spawned stuff / any time after the loading screen has dropped?
		//@TODO: Maybe just inject a random 0-1s delay in the experience load itself?

		//@待办事项：考虑不要为动态生成的内容或在加载屏幕消失之后的任何时间点进行延迟处理？
		//@待办事项：或许可以在游戏加载过程中直接插入一个 0 到 1 秒的随机延迟？

		
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::Step4_BroadcastReady));
		

		
	}
	else
	{
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::Step3_HandleExperienceLoaded));

	}
	
	
}

void UAsyncAction_ExperienceReady::Step3_HandleExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience)
{
	// 在这里可以获取到Experience 做进一步处理
	// 目前没有特定的需求,直接触发代理即可!
	Step4_BroadcastReady();
	
}

void UAsyncAction_ExperienceReady::Step4_BroadcastReady()
{
	// 触发代理!
	OnReady.Broadcast();

	
	SetReadyToDestroy();
}
