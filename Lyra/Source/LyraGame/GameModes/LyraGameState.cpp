// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameState.h"

#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Async/TaskGraphInterfaces.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "Messages/LyraVerbMessage.h"
#include "Player/LyraPlayerState.h"
#include "LyraLogChannels.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameState)

class APlayerState;
class FLifetimeProperty;

extern ENGINE_API float GAverageFPS;

ALyraGameState::ALyraGameState(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<ULyraAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	
	ExperienceManagerComponent = CreateDefaultSubobject<ULyraExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));

	ServerFPS = 0.0f;
	
}

void ALyraGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ALyraGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);

	// 初始化ASC的组件信息
	AbilitySystemComponent->InitAbilityActorInfo(/*Owner=*/ this, /*Avatar=*/ this);
}

void ALyraGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ALyraGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ServerFPS);
	// 此属性仅用于发送至回放连接
	DOREPLIFETIME_CONDITION(ThisClass, RecorderPlayerState, COND_ReplayOnly);
}

void ALyraGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetLocalRole() == ROLE_Authority)
	{
		ServerFPS = GAverageFPS;
	}
	
}

UAbilitySystemComponent* ALyraGameState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ALyraGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

void ALyraGameState::RemovePlayerState(APlayerState* PlayerState)
{
	//@TODO: This isn't getting called right now (only the 'rich' AGameMode uses it, not AGameModeBase)
	// Need to at least comment the engine code, and possibly move things around
	
	//@待办事项：目前此函数并未被调用（只有“高级”模式的 AGameMode 才会使用它，而 AGameModeBase 并不会）
	// 至少需要对引擎代码进行注释，并且可能需要对代码结构进行调整

	
	Super::RemovePlayerState(PlayerState);
}

void ALyraGameState::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
{
	// Remove inactive and bots
	// 移除未活跃用户和机器人用户
	for (int32 i = PlayerArray.Num() - 1; i >= 0; i--)
	{
		APlayerState* PlayerState = PlayerArray[i];
		if (PlayerState && (PlayerState->IsABot() || PlayerState->IsInactive()))
		{
			RemovePlayerState(PlayerState);
		}
	}

	
}

void ALyraGameState::MulticastMessageToClients_Implementation(const FLyraVerbMessage Message)
{
	if (GetNetMode() == NM_Client)
	{
		//服务器同步的信息在本地客户端广播
		//UGameplayMessageSubsystem不包含网络功能，无法和服务器通信，需要通过ActorChannel获取服务器信息，再在客户端本地调用UGameplayMessageSubsystem触发消息系统
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);

	}
}

void ALyraGameState::MulticastReliableMessageToClients_Implementation(const FLyraVerbMessage Message)
{
	MulticastMessageToClients_Implementation(Message);
}

float ALyraGameState::GetServerFPS() const
{
	return ServerFPS;
}

void ALyraGameState::SetRecorderPlayerState(APlayerState* NewPlayerState)
{
	if (RecorderPlayerState == nullptr)
	{
		// Set it and call the rep callback so it can do any record-time setup
		// 设置该值并调用回调函数，以便其能够进行任何记录时的设置操作
		RecorderPlayerState = NewPlayerState;
		OnRep_RecorderPlayerState();
	}
	else
	{
		UE_LOG(LogLyra, Warning, TEXT("SetRecorderPlayerState was called on %s but should only be called once per game on the primary user"), *GetName());
	}
}

APlayerState* ALyraGameState::GetRecorderPlayerState() const
{
	// TODO: Maybe auto select it if null?
	// 注意事项：如果为空值，或许可以自动将其选中？
	return RecorderPlayerState;
	
}

void ALyraGameState::OnRep_RecorderPlayerState()
{
	OnRecorderPlayerStateChangedEvent.Broadcast(RecorderPlayerState);
}
