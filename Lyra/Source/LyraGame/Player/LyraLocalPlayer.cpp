// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraLocalPlayer.h"


#include "AudioMixerBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
#include "CommonUserSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraLocalPlayer)

class UObject;
ULyraLocalPlayer::ULyraLocalPlayer()
{

	
}

void ULyraLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	if (ULyraSettingsLocal* LocalSettings = GetLocalSettings())
	{
		LocalSettings->OnAudioOutputDeviceChanged.AddUObject(this, &ULyraLocalPlayer::OnAudioOutputDeviceChanged);
	}
}

void ULyraLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnPlayerControllerChanged(PlayerController);
}

//这个函数的执行时机在gamemode的执行流程中，初始化LocalPlayer时调用
bool ULyraLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);

	OnPlayerControllerChanged(PlayerController);
	return bResult;
}

void ULyraLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);

	Super::InitOnlineSession();
}

void ULyraLocalPlayer::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	// Do nothing, we merely observe the team of our associated player controller
	// 不做任何操作，我们只是观察与我们关联的球员控制器所组成的团队。
}

FGenericTeamId ULyraLocalPlayer::GetGenericTeamId() const
{
	if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(PlayerController))
	{
		return ControllerAsTeamProvider->GetGenericTeamId();
	}
	else
	{
		return FGenericTeamId::NoTeam;
	}
}

FOnLyraTeamIndexChangedDelegate* ULyraLocalPlayer::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

ULyraSettingsLocal* ULyraLocalPlayer::GetLocalSettings() const
{
	return ULyraSettingsLocal::Get();
}

ULyraSettingsShared* ULyraLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		// On PC it's okay to use the sync load because it only checks the disk
		// This could use a platform tag to check for proper save support instead
		// 在个人电脑上，使用同步加载功能是没问题的，因为它只会检查磁盘。
		// 这种方式可以添加一个平台标签，以便检查是否具备正确的保存功能。
		//判断是否能够在登录前加载设置，因为如果是个人电脑可以直接从磁盘加载Setting，但是如果不是PC端，需要从云端加载设置的情况，就需要先创建一个临时设置 CreateTemporarySettings 
		bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;
		
		if (bCanLoadBeforeLogin)
		{
			SharedSettings = ULyraSettingsShared::LoadOrCreateSettings(this);
		}
		else
		{
			// We need to wait for user login to get the real settings so return temp ones
			// 我们需要等待用户登录后才能获取到真实的设置信息，所以先返回临时设置即可。
			SharedSettings = ULyraSettingsShared::CreateTemporarySettings(this);
		}

		
		
	}


	return SharedSettings;
}

void ULyraLocalPlayer::LoadSharedSettingsFromDisk(bool bForceLoad)
{
	FUniqueNetIdRepl CurrentNetId = GetCachedUniqueNetId();

	//避免重复加载，因为localsetting是引擎配置，只会加载一次，但是SharedSetting可能会频繁切换
	if (!bForceLoad && SharedSettings && CurrentNetId == NetIdForSharedSettings)
	{
		// Already loaded once, don't reload
		// 已经加载过一次，不要再重新加载了
		return;
		
	}

	ensure(ULyraSettingsShared::AsyncLoadOrCreateSettings(this,
		ULyraSettingsShared::FOnSettingsLoadedEvent::CreateUObject(this, &ULyraLocalPlayer::OnSharedSettingsLoaded)));

	
}

void ULyraLocalPlayer::OnSharedSettingsLoaded(ULyraSettingsShared* LoadedOrCreatedSettings)
{
	// The settings are applied before it gets here
	// 这些设置在到达此处之前就已经生效了，在LoadSharedSettingsFromDisk函数中执行AsyncLoadOrCreateSettings时就已经应用这些数值了
	// 执行到这里时，已经完成用户登录，初始化流程了
	if (ensure(LoadedOrCreatedSettings))
	{
		// This will replace the temporary or previously loaded object which will GC out normally
		// 这将替换临时对象或之前加载的对象，这些对象将按照正常流程进行垃圾回收。
		SharedSettings = LoadedOrCreatedSettings;

		NetIdForSharedSettings = GetCachedUniqueNetId();
	}

	
}

//这里的InAudioOutputDeviceId是从Settinglocal 中广播调用传过来的
void ULyraLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{

	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));

	/**
	 * 将音频输出设备更改为所请求的设备
	 * @参数 新设备标识 - 要更换的设备标识
	 * @参数 完成设备更换事件 - 当音频端点设备获取完毕时触发的事件
	 * 
	 */
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);

	
}

void ULyraLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
	{
	}
	
}

void ULyraLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
	// Stop listening for changes from the old controller
	// 停止监听旧控制器的更改情况
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;

	//获取旧的PlayerController的队伍信息
	if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(LastBoundPC.Get()))
	{
		OldTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	//获取新的PlayerController的队伍信息
	// Grab the current team ID and listen for future change
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	// 获取当前团队的 ID，并监听未来的变化情况
	if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(NewController))
	{
		NewTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
		LastBoundPC = NewController;
	}

	//这里先手动调用一次，马上更新队伍信息；
	//如果之后又发生了新的队伍变更，则调用 GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam) 执行代理绑定的函数
	//
	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
	
}

void ULyraLocalPlayer::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}
