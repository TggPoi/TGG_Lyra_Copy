// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEditor.h"
#include "Modules/ModuleManager.h"

class SWidget;

#define LOCTEXT_NAMESPACE "LyraEditor"

DEFINE_LOG_CATEGORY(LogLyraEditor);

/**
 * FLyraEditorModule 这里的Module直接写在LyraEditor.cpp中，因为LyraEditor.cpp是主模块，所以不需要写在LyraEditor.h中
 * 但是，如果LyraEditor.cpp不是主模块，那么就需要写在LyraEditor.h中，否则，主模块无法找到这个Module
 * 例如，LyraEditor.cpp是主模块，LyraGame.cpp不是主模块，那么LyraGame.cpp中需要写在LyraGame.h中，否则，LyraEditor.cpp无法找到这个Module
 */
class FLyraEditorModule : public FDefaultGameModuleImpl
{

	virtual void StartupModule() override
	{
	
	}



	virtual void ShutdownModule() override
	{
	
	
	}

};

IMPLEMENT_MODULE(FLyraEditorModule, LyraEditor);

#undef LOCTEXT_NAMESPACE