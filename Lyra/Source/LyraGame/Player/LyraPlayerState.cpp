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
{


}

ALyraPlayerController* ALyraPlayerState::GetLyraPlayerController() const
{
	return Cast<ALyraPlayerController>(GetOwner());
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