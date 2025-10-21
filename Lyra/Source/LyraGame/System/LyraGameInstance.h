// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
// 001 Not Finished 基本完成构建 需要在玩家控制器和玩家类都确认后 修改部分代码.
// 002 修复了玩家控制器的获取方法,修复了本地玩家类的调用加载设置方法.
#pragma once

#include "CommonGameInstance.h"

#include "LyraGameInstance.generated.h"

#define UE_API LYRAGAME_API

class ALyraPlayerController;
class UObject;


/**
 * Lyra项目的游戏实例
 */
UCLASS(MinimalAPI, Config = Game)
class ULyraGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:

	UE_API ULyraGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * 获取此设备上的主玩家控制器（其他控制器为分屏子控制器）
	 * （必须具备有效的玩家状态）
	 * @参数 bRequiresValidUniqueId - 控制器是否也必须具有有效的唯一标识符（默认值为 true，以保持历史行为）
	 * @返回 此设备上的主控制器
	 * 
	 */
	// 因为这里的返回值不一样 所以不是override
	UE_API ALyraPlayerController* GetPrimaryPlayerController() const;



	// 是否可以加入会话
	UE_API virtual bool CanJoinRequestedSession() const override;

	// 处理用户得初始化
	UE_API virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo,
		bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;


	/** 处理加密密钥的设置事宜。若游戏需要自行实现此功能，则必须在自身（可能为异步）处理完成时调用该委托函数。*/
	UE_API virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;

	/** 当客户端从服务器接收到“加密确认”控制消息时会调用此函数，通常会启用加密功能。*/
	UE_API virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;

protected:

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	/** 一个虚拟函数，旨在为自定义的游戏实例提供设置自身所需内容的机会 */
	UE_API virtual void Init() override;

	/** 一个虚拟函数，旨在为自定义的游戏实例提供在关闭时进行清理操作的机会 */
	UE_API virtual void Shutdown() override;


	// 当客户端跳转到服务器的会话关卡时,我们可以在这里重写URL,携带令牌参数
	UE_API void OnPreClientTravelToSession(FString& URL);
	
	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	/** 一个【硬编码】的加密密钥，用于测试加密代码。此密钥并不安全，请勿在实际生产环境中使用此方法！*/
	TArray<uint8> DebugTestEncryptionKey;
};