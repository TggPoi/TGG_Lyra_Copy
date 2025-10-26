// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPlayerSpawningManagerComponent.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Engine/PlayerStartPIE.h"
#include "LyraPlayerStart.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPlayerSpawningManagerComponent)

DEFINE_LOG_CATEGORY_STATIC(LogPlayerSpawning, Log, All);

ULyraPlayerSpawningManagerComponent::ULyraPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// 不需要同步 这个类是帮gamemode做spawn工作，不需要进行网络同步
	SetIsReplicatedByDefault(false);

	// 自动注册
	bAutoRegister = true;
	
	// 自动激活
	bAutoActivate = true;

	/** 如果为真，则调用虚拟的“初始化组件”方法 */
	bWantsInitializeComponent = true;

	// 需要Tick
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	
	/** 若为真，则此计时函数将初始处于启用状态，但之后仍可被关闭。*/
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
}

void ULyraPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 当有个一个关卡被添加到世界中了,我们需要处理里面的玩家出生点
	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	UWorld* World = GetWorld();
	// 绑定好世界新增的对象
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

	// 捕获场景中的玩家出生点
	for (TActorIterator<ALyraPlayerStart> It(World); It; ++It)
	{
		if (ALyraPlayerStart* PlayerStart = *It)
		{
			CachedPlayerStarts.Add(PlayerStart);
		}
	}
	
	
}

void ULyraPlayerSpawningManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

APlayerStart* ULyraPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(AController* Controller,
	const TArray<ALyraPlayerStart*>& FoundStartPoints) const
{
	if (Controller)
	{

		// 完全可用的容器
		TArray<ALyraPlayerStart*> UnOccupiedStartPoints;

		// 部分可用的容器
		TArray<ALyraPlayerStart*> OccupiedStartPoints;

		for (ALyraPlayerStart* StartPoint : FoundStartPoints)
		{
			// 现在进程检测出生点状况
			ELyraPlayerStartLocationOccupancy State = StartPoint->GetLocationOccupancy(Controller);

			switch (State)
			{
			case ELyraPlayerStartLocationOccupancy::Empty:
				UnOccupiedStartPoints.Add(StartPoint);
				break;
			case ELyraPlayerStartLocationOccupancy::Partial:
				OccupiedStartPoints.Add(StartPoint);
				break;

			}
		}

		if (UnOccupiedStartPoints.Num() > 0)
		{
			return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
		
	}


	return nullptr;
}

AActor* ULyraPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (Player)
	{
#if WITH_EDITOR
		// 这里寻找的是编辑器的专属最近生成点APlayerStartPIE
		if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player))
		{
			return PlayerStart;
		}

#endif
		// 遍历缓存中有效的玩家出生点,有可能消亡了,所以要判断下
		TArray<ALyraPlayerStart*> StarterPoints;
		for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
		{
			if (ALyraPlayerStart* Start = (*StartIt).Get())
			{
				StarterPoints.Add(Start);
			}
			else
			{
				StartIt.RemoveCurrent();
			}
			
		}

		if (APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
		{
			// start dedicated spectators at any random starting location, but they do not claim it
			// 在任何随机的起始位置安排专门的观众，但他们不会占据该位置
			if (PlayerState->IsOnlyASpectator())
			{
				if (!StarterPoints.IsEmpty())
				{
					return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)];
					
				}

				return nullptr;

			}

		}

		// 这里调用我们自定义的筛选规则 看是否可以取一个最佳的出来
		// 前提是子类实现了选择的算法，如果没有实现，就调用GetFirstRandomUnoccupiedPlayerStart 默认工具函数随机选择
		AActor* PlayerStart = OnChoosePlayerStart(Player, StarterPoints);

		if (!PlayerStart)
		{

			PlayerStart = GetFirstRandomUnoccupiedPlayerStart(Player, StarterPoints);
		}

		if (ALyraPlayerStart* LyraStart = Cast<ALyraPlayerStart>(PlayerStart))
		{
			// 对这个出生点进行宣称,并在定时器中检查宣称是否过时
			LyraStart->TryClaim(Player);
		}
		
		return PlayerStart;

		
	}

	return nullptr;
	
}

bool ULyraPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
	bool bCanRestart = true;

	// TODO Can they restart?
	// 注意事项：他们能重新启动吗？
	return bCanRestart;
}

void ULyraPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	// 先调用自己
	OnFinishRestartPlayer(NewPlayer, StartRotation);

	// 再调用蓝图拓展
	K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

void ULyraPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (ALyraPlayerStart* PlayerStart = Cast<ALyraPlayerStart>(Actor))
			{
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(PlayerStart);
				
			}
		}
	
	}

	
}

void ULyraPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	if (ALyraPlayerStart* PlayerStart = Cast<ALyraPlayerStart>(SpawnedActor))
	{
		CachedPlayerStarts.Add(PlayerStart);
	}
	
}
#if WITH_EDITOR
APlayerStart* ULyraPlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player)
{
	// Only 'Play From Here' for a player controller, bots etc. should all spawn from normal spawn points.
	// 只有玩家控制器、机器人等才应从常规的起始点生成，而“从这里开始游戏”选项则仅适用于玩家控制器。
	if (Player->IsA<APlayerController>())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				if (APlayerStart* PlayerStart = *It)
				{
					if (PlayerStart->IsA<APlayerStartPIE>())
					{
						// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
						// 当处于 PIE 模式时，如果能找到“从这里开始播放”的首个 PlayerStart 功能，则始终优先使用它。
						return PlayerStart;
					}

					
				}

			}

		}

	}
	return nullptr;
	
}
#endif
