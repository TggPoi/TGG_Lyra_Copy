// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPlayerController.h"

#include "LyraPlayerController.h"
#include "CommonInputTypeEnum.h"
#include "Components/PrimitiveComponent.h"
#include "LyraLogChannels.h"
#include "LyraCheatManager.h"
#include "LyraPlayerState.h"
#include "Camera/LyraPlayerCameraManager.h"
#include "UI/LyraHUD.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "EngineUtils.h"
#include "LyraGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Engine/GameInstance.h"
#include "AbilitySystemGlobals.h"
#include "CommonInputSubsystem.h"
#include "LyraLocalPlayer.h"
#include "GameModes/LyraGameState.h"
#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
#include "Replays/LyraReplaySubsystem.h"
#include "ReplaySubsystem.h"
#include "Development/LyraDeveloperSettings.h"
#include "GameMapsSettings.h"

#if WITH_RPC_REGISTRY
#include "Tests/LyraGameplayRpcRegistrationComponent.h"
#include "HttpServerModule.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPlayerController)

namespace Lyra
{
	namespace Input
	{
		static int32 ShouldAlwaysPlayForceFeedback = 0;
		static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("LyraPC.ShouldAlwaysPlayForceFeedback"),
		                                                                 ShouldAlwaysPlayForceFeedback,
		                                                                 TEXT(
			                                                                 "Should force feedback effects be played, even if the last input device was not a gamepad?"));
	}
}


ALyraPlayerController::ALyraPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//TODO 待完成
	// 指定相机管理类
	//PlayerCameraManagerClass = ALyraPlayerCameraManager::StaticClass();
	
#if USING_CHEAT_MANAGER
	// 指定作弊器的类
	CheatClass = ULyraCheatManager::StaticClass();
#endif // #if USING_CHEAT_MANAGER
}

ALyraPlayerState* ALyraPlayerController::GetLyraPlayerState() const
{
	return CastChecked<ALyraPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

ULyraAbilitySystemComponent* ALyraPlayerController::GetLyraAbilitySystemComponent() const
{
	const ALyraPlayerState* LyraPS = GetLyraPlayerState();
	return (LyraPS ? LyraPS->GetLyraAbilitySystemComponent() : nullptr);
}

ALyraHUD* ALyraPlayerController::GetLyraHUD() const
{
	return CastChecked<ALyraHUD>(GetHUD(), ECastCheckedType::NullAllowed);
}

bool ALyraPlayerController::TryToRecordClientReplay()
{
	// See if we should record a replay
	// 查看是否需要录制回放内容
	if (ShouldRecordClientReplay())
	{
		if (ULyraReplaySubsystem* ReplaySubsystem = GetGameInstance()->GetSubsystem<ULyraReplaySubsystem>())
		{
			APlayerController* FirstLocalPlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (FirstLocalPlayerController == this)
			{
				// If this is the first player, update the spectator player for local replays and then record
				// 如果这是第一个玩家，则为本地回放更新旁观者玩家信息，然后进行记录
				if (ALyraGameState* GameState = Cast<ALyraGameState>(GetWorld()->GetGameState()))
				{
					GameState->SetRecorderPlayerState(PlayerState);

					//@XGTODO:现在我们还没有真实的开始记录回放
					//ReplaySubsystem->RecordClientReplay(this);

					return true;
				}
			}
		}
		
	}
	
	return false;
}

bool ALyraPlayerController::ShouldRecordClientReplay()
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr &&
		World != nullptr &&
		!World->IsPlayingReplay() &&
		!World->IsRecordingClientReplay() &&
		NM_DedicatedServer != GetNetMode() &&
		IsLocalPlayerController())
	{
		//判断是否能够进行回放记录的条件：
		// 游戏实例不能为空
		// 世界不能为空
		// 不能正在播放回放
		// 不能正在记录录像
		// 不能是专属服务器
		// 必须是本地控制的玩家控制器

		//获取默认地图（lyra中的默认地图是前端选择关卡的 map
		FString DefaultMap = UGameMapsSettings::GetGameDefaultMap();
		FString CurrentMap = World->URL.Map;

#if WITH_EDITOR
		CurrentMap = UWorld::StripPIEPrefixFromPackageName(CurrentMap, World->StreamingLevelsPrefix);
#endif

		// 在前端地图选择界面的 map 禁止开启回放功能
		if (CurrentMap == DefaultMap)
		{
			// Never record demos on the default frontend map, this could be replaced with a better check for being in the main menu
			// 请勿在默认的前端地图上录制演示内容，这一操作可以改为采用更有效的检查方式，以确认是否处于主菜单界面。
			
			return false;
		}

		if (UReplaySubsystem* ReplaySubsystem = GameInstance->GetSubsystem<UReplaySubsystem>())
		{
			//判断当前是否正在记录 或者 播放回放
			if (ReplaySubsystem->IsRecording() || ReplaySubsystem->IsPlaying())
			{
				// Only one at a time
				// 一次只能一个
				return false;
			}
		}

		// If this is possible, now check the settings
		// 如果可行的话，现在就检查一下设置吧
		if (const ULyraLocalPlayer* LyraLocalPlayer = Cast<ULyraLocalPlayer>(GetLocalPlayer()))
		{

			//TODO 待完成
			//004:从本地游戏设置从读取是否自动记录回放功能
			// if (LyraLocalPlayer->GetLocalSettings()->ShouldAutoRecordReplays())
			// {
			// 	return true;
			// }
			
		}
		
	}
	return false;
	
}

void ALyraPlayerController::ServerCheat_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(LogLyra, Warning, TEXT("ServerCheat: %s"), *Msg);
		//将信息传递给客户端
		ClientMessage(ConsoleCommand(Msg));
	}
#endif // #if USING_CHEAT_MANAGER
	
}

bool ALyraPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

void ALyraPlayerController::ServerCheatAll_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	//在场景中查找所有LyraPlayerController，遍历开启作弊器，并将信息传递给客户端
	if (CheatManager)
	{
		UE_LOG(LogLyra, Warning, TEXT("ServerCheatAll: %s"), *Msg);
		for (TActorIterator<ALyraPlayerController> It(GetWorld()); It; ++It)
		{
			ALyraPlayerController* LyraPC = (*It);
			if (LyraPC)
			{
				LyraPC->ClientMessage(LyraPC->ConsoleCommand(Msg));
			}
		}
	}
#endif // #if USING_CHEAT_MANAGER
}

bool ALyraPlayerController::ServerCheatAll_Validate(const FString& Msg)
{
	return true;
}

void ALyraPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ALyraPlayerController::BeginPlay()
{
	Super::BeginPlay();

#if WITH_RPC_REGISTRY

	// 开启所有的监听
	FHttpServerModule::Get().StartAllListeners();


	// 监听的端口
	int32 RpcPort = 0;

	// 从命令行读取监听端口
	if (FParse::Value(FCommandLine::Get(), TEXT("rpcport="), RpcPort))
	{
		//@XGTODO:现在我们不需要调试远程访问服务器功能
		//远程调用组件实例创建
		/*ULyraGameplayRpcRegistrationComponent* ObjectInstance = ULyraGameplayRpcRegistrationComponent::GetInstance();
		if (ObjectInstance && ObjectInstance->IsValidLowLevel())
		{
			// 注册总是调用
			ObjectInstance->RegisterAlwaysOnHttpCallbacks();

			// 注册比赛中调用
			ObjectInstance->RegisterInMatchHttpCallbacks();
		}*/
	}


#endif

	SetActorHiddenInGame(false);
}

void ALyraPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ALyraPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Disable replicating the PC target view as it doesn't work well for replays or client-side spectating.
	// The engine TargetViewRotation is only set in APlayerController::TickActor if the server knows ahead of time that 
	// a specific pawn is being spectated and it only replicates down for COND_OwnerOnly.
	// In client-saved replays, COND_OwnerOnly is never true and the target pawn is not always known at the time of recording.
	// To support client-saved replays, the replication of this was moved to ReplicatedViewRotation and updated in PlayerTick.

	// 禁用复制 PC 目标视图，因为这对于回放或客户端旁观模式来说效果不佳。
	// 在 APlayerController::TickActor 中，只有当服务器事先知道某个特定角色正在被旁观时，才会设置引擎的 TargetViewRotation 属性，并且只有在 COND_OwnerOnly 条件下才会进行复制。
	// 在客户端保存的回放中，COND_OwnerOnly 从不为真，而且在录制时也不总是知道目标角色的具体信息。
	// 为了支持客户端保存的回放，此属性的复制已移至 ReplicatedViewRotation，并在 PlayerTick 中进行更新。


	DISABLE_REPLICATED_PROPERTY(APlayerController, TargetViewRotation);
}

void ALyraPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//编辑器开发环境下，设置是否开启作弊
#if WITH_SERVER_CODE && WITH_EDITOR
	if (GIsEditor && (InPawn != nullptr) && (GetPawn() == InPawn))
	{
		for (const FLyraCheatToRun& CheatRow : GetDefault<ULyraDeveloperSettings>()->CheatsToRun)
		{

			if (CheatRow.Phase == ECheatExecutionTime::OnPlayerPawnPossession)
			{
				ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
				
			}
		}
		
	}

	
#endif

	// 关闭自动运行，当前pawn已经被控制，不需要自动运行
	SetIsAutoRunning(false);
}

void ALyraPlayerController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
	// 确保被解除控制的棋子不会仍然是我们 ASC 的角色扮演者角色。
	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
			{
				ASC->SetAvatarActor(nullptr);
				
			}	
		}
		
	}

	//确保在解除控制之前将ASC移除
	Super::OnUnPossess();
}

void ALyraPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
	
}

void ALyraPlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();


	// When we're a client connected to a remote server, the player controller may replicate later than the PlayerState and AbilitySystemComponent.
	// However, TryActivateAbilitiesOnSpawn depends on the player controller being replicated in order to check whether on-spawn abilities should
	// execute locally. Therefore once the PlayerController exists and has resolved the PlayerState, try once again to activate on-spawn abilities.
	// On other net modes the PlayerController will never replicate late, so LyraASC's own TryActivateAbilitiesOnSpawn calls will succeed. The handling 
	// here is only for when the PlayerState and ASC replicated before the PC and incorrectly thought the abilities were not for the local player.

	// 当我们作为客户端连接到远程服务器时，玩家控制器的同步可能会晚于玩家状态和能力系统组件的同步。
	// 但是，TryActivateAbilitiesOnSpawn 函数依赖于玩家控制器已进行同步，以便检查是否应在游戏启动时本地执行初始能力。因此，一旦玩家控制器存在并已解析玩家状态，就再次尝试激活初始能力。
	// 在其他网络模式下，玩家控制器永远不会同步较晚，所以 LyraASC 的自身 TryActivateAbilitiesOnSpawn 调用将成功。此处的处理方式仅适用于当玩家状态和 ASC 在玩家控制器同步之前已进行同步，并且错误地认为这些能力并非针对本地玩家时的情况。

	if (GetWorld()->IsNetMode(NM_Client))
	{
		if (ALyraPlayerState* LyraPS = GetPlayerState<ALyraPlayerState>())
		{
			if (ULyraAbilitySystemComponent* LyraASC = LyraPS->GetLyraAbilitySystemComponent())
			{
				LyraASC->RefreshAbilityActorInfo();
				
				//@TODO :我们现在还没有定义角色出生时就需要激活的能力
				//LyraASC->TryActivateAbilitiesOnSpawn();

				
			}

			
		}

		
	}
	
}

void ALyraPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();


	
}

void ALyraPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
}

void ALyraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// If we are auto running then add some player input
	// 如果我们处于自动运行模式，则添加一些玩家操作输入，模拟玩家输入 移动
	if (GetIsAutoRunning())
	{
		if (APawn* CurrentPawn = GetPawn())
		{
			const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);

			CurrentPawn->AddMovementInput(MovementDirection, 1.0f);	
			
		}

	}

	
	

	ALyraPlayerState* LyraPlayerState = GetLyraPlayerState();

	if (PlayerCameraManager && LyraPlayerState)
	{
		APawn* TargetPawn = PlayerCameraManager->GetViewTargetPawn();

		if (TargetPawn)
		{
			// Update view rotation on the server so it replicates
			// 在服务器上更新视图旋转角度，以便实现同步复制
			// IsLocallyControlled 表示当前pawn是不是由本地主机控制（也就是不通过网络控制角色的情况）
			if (HasAuthority() || TargetPawn->IsLocallyControlled())
			{
				LyraPlayerState->SetReplicatedViewRotation(TargetPawn->GetViewRotation());
			}
			
			// Update the target view rotation if the pawn isn't locally controlled
			// 如果兵不是本地控制的，表示当前需要从服务器获取同步的数据，更新目标视图的旋转角度
			if (!TargetPawn->IsLocallyControlled())
			{
				LyraPlayerState = TargetPawn->GetPlayerState<ALyraPlayerState>();
				if (LyraPlayerState)
				{
					// Get it from the spectated pawn's player state, which may not be the same as the PC's playerstate
					// 从旁观者棋子的玩家状态中获取该信息，而该玩家状态可能与电脑玩家的状态不同。
					TargetViewRotation = LyraPlayerState->GetReplicatedViewRotation();

					
				}

				
				
			}

			
		}
	}
}

void ALyraPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	if (const ULyraLocalPlayer* LyraLocalPlayer = Cast<ULyraLocalPlayer>(InPlayer))
	{

		//TODO  待完成
		//ULyraSettingsShared* UserSettings = LyraLocalPlayer->GetSharedSettings();
		//UserSettings->OnSettingChanged.AddUObject(this, &ThisClass::OnSettingsChanged);
				
		//OnSettingsChanged(UserSettings);
	}


	
}

void ALyraPlayerController::AddCheats(bool bForce)
{
#if USING_CHEAT_MANAGER //如果设置了宏，则直接启用作弊，否则就通过传入的值来判断是否开启作弊
	Super::AddCheats(true);
#else //#if USING_CHEAT_MANAGER
	Super::AddCheats(bForce);
#endif // #else //#if USING_CHEAT_MANAGER
}

void ALyraPlayerController::UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId)
{
	if (bForceFeedbackEnabled)
	{
		if (const UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
		{
			const ECommonInputType CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
			if (Lyra::Input::ShouldAlwaysPlayForceFeedback || CurrentInputType == ECommonInputType::Gamepad || CurrentInputType == ECommonInputType::Touch)
			{
				InputInterface->SetForceFeedbackChannelValues(ControllerId, ForceFeedbackValues);
				return;
			}

			
		}

		
	}

	


	InputInterface->SetForceFeedbackChannelValues(ControllerId, FForceFeedbackValues());
}

//FPrimitiveComponentId 专门用于设置渲染相关的组件，比如隐藏组件，不渲染组件等
void ALyraPlayerController::UpdateHiddenComponents(const FVector& ViewLocation,
	TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);

	//设置是否在下一帧隐藏视图目标pawn，设置隐藏后再if 代码块的结尾重新设置为false，因为不需要每帧都隐藏 bHideViewTargetPawnNextFrame = false;
	if (bHideViewTargetPawnNextFrame)
	{
		AActor* const ViewTargetPawn = PlayerCameraManager ? Cast<AActor>(PlayerCameraManager->GetViewTarget()) : nullptr;
		if (ViewTargetPawn)
		{
			// internal helper func to hide all the components
			// 内部辅助函数，用于隐藏所有组件

			auto AddToHiddenComponents =[&OutHiddenComponents](const TInlineComponentArray<UPrimitiveComponent*>& InComponents)
			{
				// add every component and all attached children
				// 添加每个组件及其所有附属子元素

				for (UPrimitiveComponent* Comp : InComponents)
				{
					if (Comp->IsRegistered())
					{
						OutHiddenComponents.Add(Comp->GetPrimitiveSceneId());

						//判断是否附加了子组件
						for (USceneComponent* AttachedChild : Comp->GetAttachChildren())
						{
							//判断子Component的Tag属性中是否存在NoParentAutoHide标签，设置是否受父级影响，父级隐藏则隐藏子级
							static FName NAME_NoParentAutoHide(TEXT("NoParentAutoHide"));
							UPrimitiveComponent* AttachChildPC = Cast<UPrimitiveComponent>(AttachedChild);
							
							if (AttachChildPC && AttachChildPC->IsRegistered() && !AttachChildPC->ComponentTags.Contains(NAME_NoParentAutoHide))
							{
								OutHiddenComponents.Add(AttachChildPC->GetPrimitiveSceneId());
							}
						}
					}
				}
			};

			//TODO Solve with an interface.  Gather hidden components or something.
			//TODO Hiding isn't awesome, sometimes you want the effect of a fade out over a proximity, needs to bubble up to designers.

			// 请使用接口来解决这个问题。可以整合一些隐藏功能或者采取其他措施。
			// 请不要将隐藏操作视为完美解决方案，有时您可能希望在接近目标时产生逐渐淡出的效果，这需要与设计师进行沟通。
			// 例如需要做一个淡入淡出的效果，或者半透的效果

			
			// hide pawn's components
			// 隐藏棋子的组件
			// TInlineComponentArray 内联数组，避免频繁的 堆内存 分配和释放，提高性能
			TInlineComponentArray<UPrimitiveComponent*> PawnComponents;
			ViewTargetPawn->GetComponents(PawnComponents);
			AddToHiddenComponents(PawnComponents);

			//// hide weapon too
			//if (ViewTargetPawn->CurrentWeapon)
			//{
			//	TInlineComponentArray<UPrimitiveComponent*> WeaponComponents;
			//	ViewTargetPawn->CurrentWeapon->GetComponents(WeaponComponents);
			//	AddToHiddenComponents(WeaponComponents);
			//}
		}
		// we consumed it, reset for next frame
		bHideViewTargetPawnNextFrame = false;
	}

}

void ALyraPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);


	
}

void ALyraPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent())
	{
		//@XGTODO:我们还没有实现ASC处理输入句柄
		//LyraASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ALyraPlayerController::OnCameraPenetratingTarget()
{
	bHideViewTargetPawnNextFrame = true;
}

void ALyraPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	UE_LOG(LogLyraTeams, Error, TEXT("You can't set the team ID on a player controller (%s); it's driven by the associated player state"), *GetPathNameSafe(this));

}

FGenericTeamId ALyraPlayerController::GetGenericTeamId() const
{
	if (const ILyraTeamAgentInterface* PSWithTeamInterface = Cast<ILyraTeamAgentInterface>(PlayerState))
	{

		return PSWithTeamInterface->GetGenericTeamId();
	}
	
	return FGenericTeamId::NoTeam;
}

FOnLyraTeamIndexChangedDelegate* ALyraPlayerController::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void ALyraPlayerController::SetIsAutoRunning(const bool bEnabled)
{
	const bool bIsAutoRunning = GetIsAutoRunning();

	//比较实际状态 bEnabled 和 期望状态 bIsAutoRunning，如果不一致，则调用对应的函数
	if (bEnabled != bIsAutoRunning)
	{
		if (!bEnabled)
		{
			OnEndAutoRun();
		}
		else
		{
			OnStartAutoRun();
		}
		
	}
	
}

bool ALyraPlayerController::GetIsAutoRunning() const
{
	bool bIsAutoRunning = false;

	//通过tag计数判断是否要开启自动运行
	if (const ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent())
	{
		bIsAutoRunning = LyraASC->GetTagCount(LyraGameplayTags::Status_AutoRunning) > 0;

		
	}
	return bIsAutoRunning;
	
}

void ALyraPlayerController::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	//将上级变化的队伍信息传递给下级，由BroadcastOnPlayerStateChanged调用，BroadcastOnPlayerStateChanged用于玩家的PlayerState信息都发生改变时调用
	//而当前函数只是用于广播队伍信息的变化
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));

}

void ALyraPlayerController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
	// 空的，供派生类实现之用，无需处理其他所有事件（这样可以简化代码）
}

void ALyraPlayerController::BroadcastOnPlayerStateChanged()
{
	// 先调用拓展接口
	OnPlayerStateChanged();
	
	// Unbind from the old player state, if any
	// 从旧的玩家状态中解除绑定（如果存在的话）
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	
	if (LastSeenPlayerState != nullptr)
	{
		if (ILyraTeamAgentInterface* PlayerStateTeamInterface = Cast<ILyraTeamAgentInterface>(LastSeenPlayerState))
		{
			OldTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
		}
	}

	// Bind to the new player state, if any
	// 将其绑定到新的玩家状态（如果有新的玩家状态的话）
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (PlayerState != nullptr)
	{
		if (ILyraTeamAgentInterface* PlayerStateTeamInterface = Cast<ILyraTeamAgentInterface>(PlayerState))
		{
			NewTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
		}
	}
	
	// Broadcast the team change (if it really has)
	// 发布团队变更信息（如果确实有变更的话）通过获取新旧队伍ID判断是否变化
	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
	
	LastSeenPlayerState = PlayerState;
}

void ALyraPlayerController::OnSettingsChanged(ULyraSettingsShared* InSettings)
{

	//TODO 待完成
	//bForceFeedbackEnabled = InSettings->GetForceFeedbackEnabled();
}

void ALyraPlayerController::OnStartAutoRun()
{
	if (ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent())
	{
		//给自动运行状态添加一个标签计数，计数为1
		LyraASC->SetLooseGameplayTagCount(LyraGameplayTags::Status_AutoRunning, 1);

		//执行蓝图拓展函数
		K2_OnStartAutoRun();
	}
}

void ALyraPlayerController::OnEndAutoRun()
{
	if (ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent())
	{
		LyraASC->SetLooseGameplayTagCount(LyraGameplayTags::Status_AutoRunning, 0);
		K2_OnEndAutoRun();
	}
}
//////////////////////////////////////////////////////////////////////
// ALyraReplayPlayerController


void ALyraReplayPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// The state may go invalid at any time due to scrubbing during a replay
	// 由于在重放过程中进行的数据清除操作，该状态可能会随时失效。
	if (!IsValid(FollowedPlayerState))
	{
		UWorld* World = GetWorld();
		// Listen for changes for both recording and playback
		// 监听录制和播放过程中的任何变化
		if (ALyraGameState* GameState = Cast<ALyraGameState>(World->GetGameState()))
		{
			//设置GameState监听事件，当GameState的RecorderPlayerState发生变化时，调用RecorderPlayerStateUpdated函数
			if (!GameState->OnRecorderPlayerStateChangedEvent.IsBoundToObject(this))
			{
				GameState->OnRecorderPlayerStateChangedEvent.AddUObject(this, &ThisClass::RecorderPlayerStateUpdated);
				
			}

			//主动触发更新事件，上面的绑定只是用于监听后续的变化，但是如果一开始没有绑定，则需要主动触发一次更新事件，以便获取初始的RecorderPlayerState
			if (APlayerState* RecorderState = GameState->GetRecorderPlayerState())
			{
				RecorderPlayerStateUpdated(RecorderState);
			}

			
		}
		


		
	}

	
}

void ALyraReplayPlayerController::SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds)
{
	// Default behavior is to interpolate to TargetViewRotation which is set from APlayerController::TickActor but it's not very smooth
	// 默认情况下，会将值插值到“目标视图旋转”这一变量中，该变量是由 APlayerController::TickActor 方法设置的，但其过渡效果并不十分流畅。
	Super::SmoothTargetViewRotation(TargetPawn, DeltaSeconds);
}

bool ALyraReplayPlayerController::ShouldRecordClientReplay()
{
	return false;
}

void ALyraReplayPlayerController::RecorderPlayerStateUpdated(APlayerState* NewRecorderPlayerState)
{
	if (NewRecorderPlayerState)
	{
		FollowedPlayerState = NewRecorderPlayerState;

		// Bind to when pawn changes and call now
		// 当 兵种Pawn 发生变化时绑定此操作，并立即执行
		NewRecorderPlayerState->OnPawnSet.AddUniqueDynamic(this, &ALyraReplayPlayerController::OnPlayerStatePawnSet);

		OnPlayerStatePawnSet(NewRecorderPlayerState, NewRecorderPlayerState->GetPawn(), nullptr);


		
	}

	
}

void ALyraReplayPlayerController::OnPlayerStatePawnSet(APlayerState* ChangedPlayerState, APawn* NewPlayerPawn,
	APawn* OldPlayerPawn)
{
	if (ChangedPlayerState == FollowedPlayerState)
	{
		//切换pawn视角
		SetViewTarget(NewPlayerPawn);
	}

	
}
