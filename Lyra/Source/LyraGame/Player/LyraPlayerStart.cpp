// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPlayerStart.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"

ALyraPlayerStart::ALyraPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

ELyraPlayerStartLocationOccupancy ALyraPlayerStart::GetLocationOccupancy(AController* const ControllerPawnToFit) const
{
	UWorld* const World = GetWorld();
	if (HasAuthority() && World)
	{
		if (AGameModeBase* AuthGameMode = World->GetAuthGameMode())
		{
			TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			
			FVector ActorLocation = GetActorLocation();
			const FRotator ActorRotation = GetActorRotation();
			/** 如果演员在测试位置处会因受到阻碍物的阻挡而被迫移动，则返回 true 。 同时还会返回一个可能使测试位置不再受阻碍的调整建议值。*/
			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation, nullptr))
			{
				// 可以完美的放进去
				return ELyraPlayerStartLocationOccupancy::Empty;
			}
			/**
			 * 尝试找到一个可接受且不会发生碰撞的位置来放置 TestActor，使其尽可能靠近 PlaceLocation。要求 PlaceLocation 必须是关卡内有效的位置。
			 * 如果找到了一个没有阻碍碰撞的合适位置，则返回 true，此时 PlaceLocation 会被更新为新的无碰撞位置。
			 * 如果未找到合适的位置，则返回 false，此时 PlaceLocation 不会改变。
			 * 
			 */
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{

				// 需要移动部分位置
				return ELyraPlayerStartLocationOccupancy::Partial;
			}
		
		}
		
	}
	

	// 被完全阻塞了
	return ELyraPlayerStartLocationOccupancy::Full;
}

bool ALyraPlayerStart::IsClaimed() const
{
	return ClaimingController != nullptr;
}

bool ALyraPlayerStart::TryClaim(AController* OccupyingController)
{
	if (OccupyingController != nullptr && !IsClaimed())
	{
		ClaimingController = OccupyingController;
		if (UWorld* World = GetWorld())
		{
			// 这里需要有一个宣称判断
			World->GetTimerManager().SetTimer(ExpirationTimerHandle,
				FTimerDelegate::CreateUObject(this, &ALyraPlayerStart::CheckUnclaimed), ExpirationCheckInterval, true);
			
		}
	}

	return false;
}

void ALyraPlayerStart::CheckUnclaimed()
{
	if (ClaimingController != nullptr && ClaimingController->GetPawn() != nullptr && GetLocationOccupancy(ClaimingController) == ELyraPlayerStartLocationOccupancy::Empty)
	{
		// 证明已经使用完毕啦!
		ClaimingController = nullptr;
		
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
		}

		
	}

	
}
