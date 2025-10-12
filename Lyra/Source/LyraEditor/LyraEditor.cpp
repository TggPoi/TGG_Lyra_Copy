// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEditor.h"

#include "AbilitySystemGlobals.h"
#include "DataValidationModule.h"
#include "Development/LyraDeveloperSettings.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GameEditorStyle.h"
#include "GameModes/LyraExperienceManager.h"
#include "GameplayAbilitiesEditorModule.h"
#include "GameplayCueInterface.h"
#include "GameplayCueNotify_Burst.h"
#include "GameplayCueNotify_BurstLatent.h"
#include "GameplayCueNotify_Looping.h"
#include "Private/AssetTypeActions_LyraContextEffectsLibrary.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "UObject/UObjectIterator.h"
#include "UnrealEdGlobals.h"
#include "Validation/EditorValidator.h"

class SWidget;

#define LOCTEXT_NAMESPACE "LyraEditor"

DEFINE_LOG_CATEGORY(LogLyraEditor);

/**
 * FLyraEditorModule
 * FLyraEditorModule 这里的Module直接写在LyraEditor.cpp中，因为LyraEditor.cpp是主模块，所以不需要写在LyraEditor.h中
 * 但是，如果LyraEditor.cpp不是主模块，那么就需要写在LyraEditor.h中，否则，主模块无法找到这个Module
 * 例如，LyraEditor.cpp是主模块，LyraGame.cpp不是主模块，那么LyraGame.cpp中需要写在LyraGame.h中，否则，LyraEditor.cpp无法找到这个Module
 */
class FLyraEditorModule : public FDefaultGameModuleImpl
{
	//必须要在这里进行定义 否则用不了ThisClass
	typedef FLyraEditorModule ThisClass;
		
	virtual void StartupModule() override
	{
		if (!IsRunningGame())
		{
			FEditorDelegates::BeginPIE.AddRaw(this, &ThisClass::OnBeginPIE);
			FEditorDelegates::EndPIE.AddRaw(this, &ThisClass::OnEndPIE);
		}
	}



	virtual void ShutdownModule() override
	{
	
	
	}

	void OnBeginPIE(bool bIsSimulating)
	{
		ULyraExperienceManager* ExperienceManager = GEngine->GetEngineSubsystem<ULyraExperienceManager>();
		check(ExperienceManager);
		ExperienceManager->OnPlayInEditorBegun();
	}
	
	void OnEndPIE(bool bIsSimulating)
	{
		
	}
	
};

IMPLEMENT_MODULE(FLyraEditorModule, LyraEditor);

#undef LOCTEXT_NAMESPACE