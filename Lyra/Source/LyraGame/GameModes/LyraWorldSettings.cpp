// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Misc/UObjectToken.h"
#include "Logging/MessageLog.h"
#include "LyraLogChannels.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraWorldSettings)

ALyraWorldSettings::ALyraWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	
}

FPrimaryAssetId ALyraWorldSettings::GetDefaultGameplayExperience() const
{

	// This identifies an object as a "primary" asset that can be searched for by the AssetManager and used in various tools
	// 这一标识表明该对象属于“主要”资产类别，可被资产管理器进行搜索，并能在各种工具中使用。
	FPrimaryAssetId Result;

	if (!DefaultGameplayExperience.IsNull())
	{
		//通过资产管理器加载软应用,检查是否可以获取到该资产.
		Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath());

		if (!Result.IsValid())
		{
			UE_LOG(LogLyraExperience, Error, TEXT("%s.DefaultGameplayExperience is %s but that failed to resolve into an asset ID (you might need to add a path to the Asset Rules in your game feature plugin or project settings"),
				*GetPathNameSafe(this), *DefaultGameplayExperience.ToString());
			
		}
	
	}
	
	return Result;
}


#if WITH_EDITOR
void ALyraWorldSettings::CheckForErrors()
{
	Super::CheckForErrors();

	FMessageLog MapCheck("MapCheck");

	//检查以下是否使用的是Lyra自定义的玩家起始点
	for (TActorIterator<APlayerStart> PlayerStartIt(GetWorld()); PlayerStartIt; ++PlayerStartIt)
	{
		APlayerStart* PlayerStart = *PlayerStartIt;

		//如果玩家起始点PlayerStart 不是Lyra自定义的类结构,则发出警告.
		if (IsValid(PlayerStart) && PlayerStart->GetClass() == APlayerStart::StaticClass())
		{
			MapCheck.Warning()
				->AddToken(FUObjectToken::Create(PlayerStart))
				->AddToken(FTextToken::Create(FText::FromString("is a normal APlayerStart, replace with ALyraPlayerStart.")));
		}

	}
	
	//@TODO: Make sure the soft object path is something that can actually be turned into a primary asset ID (e.g., is not pointing to an experience in an unscanned directory)
	//@待办事项：确保软对象路径能够实际转换为一个主资产标识符（例如，不能指向未扫描目录中的体验）
	
}

#endif
