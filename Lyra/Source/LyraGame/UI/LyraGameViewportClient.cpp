// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameViewportClient.h"

#include "CommonUISettings.h"
#include "ICommonUIModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameViewportClient)

class UGameInstance;

namespace GameViewportTags
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_Input_HardwareCursor, "Platform.Trait.Input.HardwareCursor");
}


ULyraGameViewportClient::ULyraGameViewportClient()
: Super(FObjectInitializer::Get())
{
	
}

void ULyraGameViewportClient::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance,
	bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);

	// We have software cursors set up in our project settings for console/mobile use, but on desktop we're fine with
	// the standard hardware cursors
	// 我们在项目设置中为桌面端和移动端设置了软件鼠标指针，但在台式机上，我们则使用标准的硬件鼠标指针即可。


	const bool UseHardwareCursor = ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(GameViewportTags::TAG_Platform_Trait_Input_HardwareCursor);

	/**
	 * 设定是否使用软件光标控件。
	 * 若未设置任何软件光标控件，则此设置将毫无实际作用。
	 * 
	 */
	SetUseSoftwareCursorWidgets(!UseHardwareCursor);


	
	
}
