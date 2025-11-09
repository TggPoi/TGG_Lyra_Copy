// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameInstance.h"


#include "CommonSessionSubsystem.h"
#include "CommonUserSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "HAL/IConsoleManager.h"
#include "LyraGameplayTags.h"
#include "Misc/Paths.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraLocalPlayer.h"
#include "GameFramework/PlayerState.h"

#if UE_WITH_DTLS //判断当前是否开启了DTLS
#include "DTLSCertificate.h"
#include "DTLSCertStore.h"
#include "DTLSHandlerComponent.h"
#include "Misc/FileHelper.h"
#endif // UE_WITH_DTLS

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameInstance)

/**
 * 控制生成证书的命令
 */
namespace Lyra
{
	// 开启加密验证这个功能
	static bool bTestEncryption = false;

	//切换加密功能是否开启
	static FAutoConsoleVariableRef CVarLyraTestEncryption(
	TEXT("Lyra.TestEncryption"),
	bTestEncryption,
	TEXT("If true, clients will send an encryption token with their request to join the server and attempt to encrypt the connection using a debug key. This is NOT SECURE and for demonstration purposes only."),
	ECVF_Default);

#if UE_WITH_DTLS
	
	// 是否使用DTLS来进行加密验证【如果要使用就需要配置好证书，反之就只验证Token】
	static bool bUseDTLSEncryption = false;
	
	static FAutoConsoleVariableRef CVarLyraUseDTLSEncryption(
		TEXT("Lyra.UseDTLSEncryption"),
		bUseDTLSEncryption,
		TEXT("Set to true if using Lyra.TestEncryption and the DTLS packet handler."),
		ECVF_Default);

	
	/* Intended for testing with multiple game instances on the same device (desktop builds) */
	/* 用于在同一设备上对多个游戏实例进行测试（适用于桌面版本） */
	// 当使用DTLS进行加密验证时,可以选择开启指纹附加,每一个链接都会根据这个加密Token生成专属的证书.这个指纹流程的话应该通过网络方式获取.
	static bool bTestDTLSFingerprint = false;
	
	static FAutoConsoleVariableRef CVarLyraTestDTLSFingerprint(
		TEXT("Lyra.TestDTLSFingerprint"),
		bTestDTLSFingerprint,
		TEXT("If true and using DTLS encryption, generate unique cert per connection and fingerprint will be written to file to simulate passing through an online service."),
		ECVF_Default);

//没有证书的情况下,使用这个命令来生成一个证书	
#if !UE_BUILD_SHIPPING
	static FAutoConsoleCommandWithWorldAndArgs CmdGenerateDTLSCertificate(
	TEXT("GenerateDTLSCertificate"),
	TEXT("Generate a DTLS self-signed certificate for testing and export to PEM."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& InArgs, UWorld* InWorld)// 生成证书需要执行的lambda函数
	{
		if (InArgs.Num() == 1)
		{
			const FString& CertName = InArgs[0];

			FTimespan CertExpire = FTimespan::FromDays(365);
			TSharedPtr<FDTLSCertificate> Cert = FDTLSCertStore::Get().CreateCert(CertExpire, CertName);
			if (Cert.IsValid())
			{
				const FString CertPath = FPaths::ProjectContentDir() / TEXT("DTLS") / FPaths::MakeValidFileName(FString::Printf(TEXT("%s.pem"), *CertName));

				//导出证书到指定路径
				if (!Cert->ExportCertificate(CertPath))
				{
					UE_LOG(LogTemp, Error, TEXT("GenerateDTLSCertificate: Failed to export certificate."));
				}

				
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GenerateDTLSCertificate: Failed to generate certificate."));	
			}

			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GenerateDTLSCertificate: Invalid argument(s)."));
		}

		
	}));

	
#endif // UE_BUILD_SHIPPING
#endif // UE_WITH_DTLS
	
}




ULyraGameInstance::ULyraGameInstance(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

ALyraPlayerController* ULyraGameInstance::GetPrimaryPlayerController() const
{
	//
	return Cast<ALyraPlayerController>(Super::GetPrimaryPlayerController(false));

}

bool ULyraGameInstance::CanJoinRequestedSession() const
{
	// Temporary first pass:  Always return true
	// This will be fleshed out to check the player's state

	// 临时初步处理：始终返回真值
	// 这部分将被进一步完善，以检查玩家的状态

	//父类源码也是始终返回true
	if (!Super::CanJoinRequestedSession())
	{
		return false;
	}
	return true;
}

void ULyraGameInstance::HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error,
	ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	Super::HandlerUserInitialized(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);

	
	// If login succeeded, tell the local player to load their settings
	// 如果登录成功，就告知本地玩家加载他们的设置
	if (bSuccess && ensure(UserInfo))
	{
	
		ULyraLocalPlayer* LocalPlayer = Cast<ULyraLocalPlayer>(GetLocalPlayerByIndex(UserInfo->LocalPlayerIndex));
	
		// There will not be a local player attached to the dedicated server user
		// 专用服务器用户不会关联本地玩家。
		if (LocalPlayer)
		{
			
			LocalPlayer->LoadSharedSettingsFromDisk();
		}
	}
}

void ULyraGameInstance::ReceivedNetworkEncryptionToken(const FString& EncryptionToken,
	const FOnEncryptionKeyResponse& Delegate)
{
	// This is a simple implementation to demonstrate using encryption for game traffic using a hardcoded key.
	// For a complete implementation, you would likely want to retrieve the encryption key from a secure source,
	// such as from a web service over HTTPS. This could be done in this function, even asynchronously - just
	// call the response delegate passed in once the key is known. The contents of the EncryptionToken is up to the user,
	// but it will generally contain information used to generate a unique encryption key, such as a user and/or session ID.

	// 这是一个简单的示例，用于展示如何使用加密技术对游戏数据进行传输，所使用的加密密钥为硬编码形式。
	// 若要实现完整的功能，您可能需要从安全来源（例如通过 HTTPS 连接的网络服务）获取加密密钥。
	// 这种操作可以在本函数中完成，甚至可以异步进行——一旦获取到密钥，只需调用传入的响应委托即可。加密令牌的内容由用户决定，但通常会包含用于生成唯一加密密钥的信息，例如用户和/或会话 ID。

	// 先准备一个无效回复
	FEncryptionKeyResponse Response(EEncryptionResponse::Failure, TEXT("Unknown encryption failure"));

	// 校验是否传了令牌 不可为空!
	if (EncryptionToken.IsEmpty())
	{
		Response.Response = EEncryptionResponse::InvalidToken;
		Response.ErrorMsg = TEXT("Encryption token is empty.");
	}
	else
	{
		// 确认有令牌了
#if UE_WITH_DTLS
	if (Lyra::bUseDTLSEncryption)
	{
		// 准备一个证书的智能指针
		TSharedPtr<FDTLSCertificate> Cert;

		// 是否要采用额外采用指纹验证
		if (Lyra::bTestDTLSFingerprint)
		{
			// 如果要采用指纹验证的话,需要创建每次创建一个新的证书
			// Generate server cert for this identifier, post the fingerprint
			// 为该标识生成服务器证书，并发布指纹信息
			// 设置一个过期时间
			FTimespan CertExpire = FTimespan::FromHours(4);

			// 根据URL 传上来的令牌 创建一个证书
			Cert = FDTLSCertStore::Get().CreateCert(CertExpire, EncryptionToken);
		}
		else
		{
			// Load cert from disk for testing purposes (never in production)
			// 从磁盘加载证书以用于测试目的（在生产环境中绝不可使用）
			const FString CertPath = FPaths::ProjectContentDir() / TEXT("DTLS") / TEXT("LyraTest.pem");

			// 看看能不能找到对应的证书
			Cert = FDTLSCertStore::Get().GetCert(EncryptionToken);

			// 找不到的话,就用我们本地通过命令行生成的
			if (!Cert.IsValid())
			{
				Cert = FDTLSCertStore::Get().ImportCert(CertPath, EncryptionToken);
			}
	
		}

		// 证书必须是可用的 否则认为验证失败了
		if (Cert.IsValid())
		{
			if (Lyra::bTestDTLSFingerprint)
			{
				// Fingerprint should be posted to a secure web service for discovery
				// Writing to disk for local testing
				// 指纹信息应发送至安全的网络服务进行识别
				// 为了测试,服务端写入磁盘以便于本地测试
				// 为了测试,客户端读取磁盘以便用于测试
				TArrayView<const uint8> Fingerprint = Cert->GetFingerprint();
				
				FString DebugFile = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("DTLS")) / FPaths::MakeValidFileName(EncryptionToken) + TEXT("_server.txt");

				FString FingerprintStr = BytesToHex(Fingerprint.GetData(), Fingerprint.Num());

				FFileHelper::SaveStringToFile(FingerprintStr, *DebugFile);

			}

			// Server currently only needs the identifier
			// 服务器目前只需要标识符即可 因为我们是服务器!
			Response.EncryptionData.Identifier = EncryptionToken;
			Response.EncryptionData.Key = DebugTestEncryptionKey;

			Response.Response = EEncryptionResponse::Success;

		}
		else
		{
			Response.Response = EEncryptionResponse::Failure;
			Response.ErrorMsg = TEXT("Unable to obtain certificate.");
		}



		
	}
	else
#endif // UE_WITH_DTLS
		{
			// 因为这里没有采用DLTS的加密验证,并且使用的调试令牌参数为EncryptionToken=1
			// 所以我们都是使用同样的密钥进行加密即可!
			Response.Response = EEncryptionResponse::Success;

			Response.EncryptionData.Key = DebugTestEncryptionKey;
		}
		
	}

	


	Delegate.ExecuteIfBound(Response);
	
}

void ULyraGameInstance::ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate)
{
	// This is a simple implementation to demonstrate using encryption for game traffic using a hardcoded key.
	// For a complete implementation, you would likely want to retrieve the encryption key from a secure source,
	// such as from a web service over HTTPS. This could be done in this function, even asynchronously - just
	// call the response delegate passed in once the key is known.

	// 这是一个简单的示例，用于展示如何使用加密技术对游戏数据进行传输，并使用一个固定的密钥进行加密。
	// 若要实现完整的功能，您可能需要从安全的来源（例如通过 HTTPS 连接的网络服务）获取加密密钥。
	// 这种操作可以在本函数中完成，甚至可以异步进行——一旦获取到密钥，只需调用传入的响应委托即可。

	// 准备一个用于返回的回应
	FEncryptionKeyResponse Response;

#if UE_WITH_DTLS
	if (Lyra::bUseDTLSEncryption)
	{
		Response.Response = EEncryptionResponse::Failure;

		APlayerController* const PlayerController = GetFirstLocalPlayerController();


		if (PlayerController && PlayerController->PlayerState && PlayerController->PlayerState->GetUniqueId().IsValid())
		{

			const FUniqueNetIdRepl& PlayerUniqueId = PlayerController->PlayerState->GetUniqueId();

			// 这个令牌是我们在ClientTravel传入重写的令牌!
			// Ideally the encryption token is passed in directly rather than having to attempt to rebuild it
			// 最理想的情况是，加密令牌EncryptionToken应直接传递进来，而无需在这里的代码中尝试重新构建它。
			const FString EncryptionToken = PlayerUniqueId.ToString();

			// 写入标识符
			Response.EncryptionData.Identifier = EncryptionToken;

			// Server's fingerprint should be pulled from a secure service
			// 服务器的指纹信息应从安全服务中获取
			// 这里的安全服务就说 我们架构的第三方服务
			if (Lyra::bTestDTLSFingerprint)
			{
				// But for testing purposes...
				// 但出于测试目的……
				FString DebugFile = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("DTLS")) / FPaths::MakeValidFileName(EncryptionToken) + TEXT("_server.txt");
				FString FingerprintStr;
				FFileHelper::LoadFileToString(FingerprintStr, *DebugFile);
				
				/** 密码学知识，没必要深究，这里就是把十六进制字符串转换为二进制数据
				 * 为什么这里要除以2?
				 *
				 * 在UE5的这段代码中，FingerprintStr.Len() / 2 的原因与十六进制字符串的表示方式有关。以下是详细解释：

				 * 十六进制字符串的特性：

				 * 每个字节(byte)用2个十六进制字符表示（例如：FF表示一个字节的值为255）。

				 * 因此，字符串长度是实际二进制数据长度的两倍。
				 * 代码逻辑：
				 * FingerprintStr 是从文件读取的十六进制字符串（如"A1B2C3..."）。
				 * HexToBytes 函数会将这个十六进制字符串转换为二进制数据（每个2字符组合转换为1字节）。
				 * HexToBytes 函数会将这个十六进制字符串转换为二进制数据（每个2字符组合转换为1字节）。
				 * 示例：
				 * 如果文件内容是"A1B2"（长度=4），则需要2字节的缓冲区（4/2=2），转换后得到[0xA1, 0xB2]。
				 * AddUninitialized的作用：
				 * 预先分配正确大小的缓冲区（避免动态调整大小开销）。
				 * 参数是元素数量（字节数），而不是字符数。
				 * 这种处理方式在密码学/网络传输中很常见，因为指纹/哈希值通常以十六进制字符串形式存储，但实际传输时需要二进制格式。
				 * 
				 */
				Response.EncryptionData.Fingerprint.AddUninitialized(FingerprintStr.Len() / 2);
				HexToBytes(FingerprintStr, Response.EncryptionData.Fingerprint.GetData());
				
			}
			else
			{
				// Pulling expected fingerprint from disk for testing, this should come from a secure service
				// 正在从磁盘中读取预期的指纹以进行测试，该指纹应来自一个安全的服务。
				// 这个证书 先从引擎里面读取 看看有没有 没有就用我们自己生成的
				const FString CertPath = FPaths::ProjectContentDir() / TEXT("DTLS") / TEXT("LyraTest.pem");
				
				TSharedPtr<FDTLSCertificate> Cert = FDTLSCertStore::Get().GetCert(EncryptionToken);
				
				if (!Cert.IsValid())
				{
					Cert = FDTLSCertStore::Get().ImportCert(CertPath, EncryptionToken);
				}

				if (Cert.IsValid())
				{
					// 获取指纹
					TArrayView<const uint8> Fingerprint = Cert->GetFingerprint();
					
					// 设置指纹
					Response.EncryptionData.Fingerprint = Fingerprint;
				}
				else
				{
					// 如果无法获取到证书 那么久失败了
					
					Response.Response = EEncryptionResponse::Failure;
					Response.ErrorMsg = TEXT("Unable to obtain certificate.");
				}
			}

			// 设置加密密钥
			Response.EncryptionData.Key = DebugTestEncryptionKey;

			// 成功设置
			Response.Response = EEncryptionResponse::Success;
			
		}
	}
	else
#endif 
	
	{
		//如果没有使用DTLS 我们就只需要加密的这个密钥就可以了.
		Response.Response = EEncryptionResponse::Success;
		Response.EncryptionData.Key = DebugTestEncryptionKey;
	}

	Delegate.ExecuteIfBound(Response);
}

void ULyraGameInstance::Init()
{
	Super::Init();

	// Register our custom init states
	// 注册我们的自定义初始化状态
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	// 注册一个全局的状态.
	if (ensure(ComponentManager))
	{
		//设置两个状态的关联关系，例如下面InitState_Spawned状态可以从空状态进入
		ComponentManager->RegisterInitState(LyraGameplayTags::InitState_Spawned, false, FGameplayTag());
		//设置两个状态的关联关系，例如下面InitState_DataAvailable状态可以从InitState_Spawned状态进入
		ComponentManager->RegisterInitState(LyraGameplayTags::InitState_DataAvailable, false, LyraGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(LyraGameplayTags::InitState_DataInitialized, false, LyraGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(LyraGameplayTags::InitState_GameplayReady, false, LyraGameplayTags::InitState_DataInitialized);

	}

	// Initialize the debug key with a set value for AES256. This is not secure and for example purposes only.
	// 为 AES256 加密算法初始化调试密钥，并设置一个固定值。此操作并不安全，仅用于演示目的。
	DebugTestEncryptionKey.SetNum(32);

	for (int32 i = 0; i < DebugTestEncryptionKey.Num(); ++i)
	{
		DebugTestEncryptionKey[i] = uint8(i);
	}
	
	//在客户端访问服务器URL前重写URL 添加令牌校验参数
	if (UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
	{
		//【官方提供的网络服务插件SessionSubsystem】
		SessionSubsystem->OnPreClientTravelEvent.AddUObject(this, &ULyraGameInstance::OnPreClientTravelToSession);
	}
	
}

void ULyraGameInstance::Shutdown()
{
	if (UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
	{
		SessionSubsystem->OnPreClientTravelEvent.RemoveAll(this);
	}
	
	Super::Shutdown();
}

void ULyraGameInstance::OnPreClientTravelToSession(FString& URL)
{
	// Add debug encryption token if desired.
	// 如果需要的话，添加调试加密令牌。

	if (Lyra::bTestEncryption)
	{
#if UE_WITH_DTLS
		// 使用DTLS这种加密方式
		if (Lyra::bUseDTLSEncryption)
		{
			// 获取本地的玩家控制器
			APlayerController* const PlayerController = GetFirstLocalPlayerController();

			if (PlayerController && PlayerController->PlayerState && PlayerController->PlayerState->GetUniqueId().IsValid())
			{
				// 获取玩家控制器的唯一ID作为令牌.
				const FUniqueNetIdRepl& PlayerUniqueId = PlayerController->PlayerState->GetUniqueId();
				const FString EncryptionToken = PlayerUniqueId.ToString();
				
				URL += TEXT("?EncryptionToken=") + EncryptionToken;

			}
		}
	else
#endif // UE_WITH		
		{
			// This is just a value for testing/debugging, the server will use the same key regardless of the token value.
			// But the token could be a user ID and/or session ID that would be used to generate a unique key per user and/or session, if desired.

			// 这只是一个用于测试/调试的值，服务器无论收到何种令牌值，都会使用相同的密钥。
			// 但该令牌可以是用户 ID 和/或会话 ID，如果需要的话，这些值将用于为每个用户和/或会话生成唯一的密钥。
			URL += TEXT("?EncryptionToken=1");
		}
	}
	
}

