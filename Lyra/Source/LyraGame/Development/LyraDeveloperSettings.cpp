// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraDeveloperSettings.h"
#include "Misc/App.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraDeveloperSettings)

#define LOCTEXT_NAMESPACE "LyraCheats"

ULyraDeveloperSettings::ULyraDeveloperSettings()
{
	
}

FName ULyraDeveloperSettings::GetCategoryName() const
{
	// 设置类别名称为项目名 方便统一管理
	return FApp::GetProjectName();
}
#if WITH_EDITOR
void ULyraDeveloperSettings::OnPlayInEditorStarted() const
{
	// Show a notification toast to remind the user that there's an experience override set
	// 显示一条通知提示信息，提醒用户已设置体验覆盖选项
	if (ExperienceOverride.IsValid())
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("ExperienceOverrideActive", "Developer Settings Override\nExperience {0}"),
			FText::FromName(ExperienceOverride.PrimaryAssetName)
		));
		
		Info.ExpireDuration = 2.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		
	}
	
}

void ULyraDeveloperSettings::ApplySettings()
{
}
#endif

//这里三个函数和上面分开使用 WITH_EDITOR包裹 ，因为下面这三个函数来自UObject，和上面的函数来源不一样
#if WITH_EDITOR
void ULyraDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//让Setting类主动去改变关联的资产属性，而不是让资产来监听
	ApplySettings();
}

void ULyraDeveloperSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);

	ApplySettings();
}

void ULyraDeveloperSettings::PostInitProperties()
{
	//临时处理一下,确保这个消息路由模块已经加载进来了,这样才能识别到命令行变量
	FModuleManager::Get().LoadModuleChecked(TEXT("GameplayMessageRuntime"));
	
	Super::PostInitProperties();

	ApplySettings();
}
#endif
#undef LOCTEXT_NAMESPACE