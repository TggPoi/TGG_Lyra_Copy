// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPlayerState.h"


#include "AbilitySystem/Attributes/LyraCombatSet.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraPawnData.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/LyraExperienceManagerComponent.h"
//@TODO: Would like to isolate this a bit better to get the pawn data in here without this having to know about other stuff
//@待办事项：希望能将这部分内容进一步独立出来，以便在此处获取棋子数据，而无需让这部分代码去了解其他内容。
#include "GameModes/LyraGameMode.h"
#include "LyraLogChannels.h"
#include "LyraPlayerController.h"
#include "Messages/LyraVerbMessage.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;

ALyraPlayerState::ALyraPlayerState(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	, MyPlayerConnectionType(ELyraPlayerConnectionType::Player)
{

	MyTeamID = FGenericTeamId::NoTeam;
	MySquadID = INDEX_NONE;
}

void ALyraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	/** 此属性是否采用推送模型。请参阅 PushModel.h 文件 */
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyTeamID, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MySquadID, SharedParams);
	
	// 跳过拥有者
	SharedParams.Condition = ELifetimeCondition::COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, SharedParams);
	
}

ALyraPlayerController* ALyraPlayerState::GetLyraPlayerController() const
{
	return Cast<ALyraPlayerController>(GetOwner());
}

void ALyraPlayerState::Reset()
{
	Super::Reset();
}

void ALyraPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);
	
	//@XGTODO:这块内容需要等角色初始化的过程时完善 等待所有组件都初始化完毕，当一个组件初始化完成时要发出通知，然后检查是否还有组件没有初始化完成，如果有，则等待，如果没有，则继续初始化下一个组件
	/*if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}*/

	
}

void ALyraPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//@TODO: Copy stats
	//@待办事项：复制统计数据
}

void ALyraPlayerState::OnDeactivated()
{
	// 是否需要设置 失去链接之后摧毁PlayerState
	bool bDestroyDeactivatedPlayerState = false;
	
	switch (GetPlayerConnectionType())
	{
	case ELyraPlayerConnectionType::Player:
	case ELyraPlayerConnectionType::InactivePlayer:
		//@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
		// (e.g., for long running servers where they might build up if lots of players cycle through)

		//@待办事项：询问体验，我们是应该立即驱逐正在断开连接的玩家，还是让他们继续留在系统中
		// （例如，在长时间运行的服务器中，如果有很多玩家频繁上线和下线，他们可能会累积起来）
		bDestroyDeactivatedPlayerState = true;
				
		break;
		
	default:
		bDestroyDeactivatedPlayerState = true;
		break;
		
		
	}
	
	SetPlayerConnectionType(ELyraPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState)
	{
		Destroy();
	}

}

void ALyraPlayerState::OnReactivated()
{
	if (GetPlayerConnectionType() == ELyraPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(ELyraPlayerConnectionType::Player);
	}
	
}

void ALyraPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (HasAuthority())
	{
		const FGenericTeamId OldTeamID = MyTeamID;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyTeamID, this);
		
		MyTeamID = NewTeamID;
		
		ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
	}
	else
	{
		UE_LOG(LogLyraTeams, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
	}
}

FGenericTeamId ALyraPlayerState::GetGenericTeamId() const
{
	return MyTeamID;
}

FOnLyraTeamIndexChangedDelegate* ALyraPlayerState::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void ALyraPlayerState::SetPlayerConnectionType(ELyraPlayerConnectionType NewType)
{
	// 标记指定的属性为“脏”状态，需提供类名、属性名以及对象。若该属性或类无效，则此操作将无法编译通过。
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
	MyPlayerConnectionType = NewType;
	
}

void ALyraPlayerState::SetSquadID(int32 NewSquadID)
{
	// 只允许在权威性角色上进行切换
	if (HasAuthority())
	{
		// 标记指定的属性为“脏”状态，需提供类名、属性名以及对象。若该属性或类无效，则此操作将无法编译通过。
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MySquadID, this);

		MySquadID = NewSquadID;
	}
	
}

void ALyraPlayerState::ClientBroadcastMessage_Implementation(const FLyraVerbMessage Message)
{
	// This check is needed to prevent running the action when in standalone mode
	// 此检查是为了防止在独立模式下执行该操作而设置的。
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
	
}

void ALyraPlayerState::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
	// 通知队伍发生了改变
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
	
}

void ALyraPlayerState::OnRep_MySquadID()
{
	//@TODO: Let the squad subsystem know (once that exists)
	//@待办事项：一旦有战队子系统，就通知该系统。

	// 目前没有战队子系统
	
}