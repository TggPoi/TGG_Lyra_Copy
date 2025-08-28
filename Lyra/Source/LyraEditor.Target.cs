// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraEditorTarget : TargetRules
{
	public LyraEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		
		ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor" });
		
		// Build all the modules that are valid for this target type. Used for CIS and making installed engine builds.
		// 构建适用于此目标类型的所有模块。用于 CIS（配置信息标准）以及生成已安装的引擎版本。
		if (!bBuildAllModules)
		{
			// Used to override the behavior controlling whether UCLASSes and USTRUCTs are allowed to have native pointer members, if disallowed they will be a UHT error and should be substituted with TObjectPtr members instead.
			// 用于覆盖控制 UCLASS 和 USTRUCT 是否允许拥有原生指针成员的设置。如果不允许，那么就会出现 UHT 错误，并且应当用 TObjectPtr 成员来替代。	
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;		
			
		}
		
		LyraGameTarget.ApplySharedLyraTargetSettings(this);
		
		// This is used for touch screen development along with the "Unreal Remote 2" app
		// 此功能用于触屏设备开发，同时与“虚幻远程 2”应用程序配合使用。
		EnablePlugins.Add("RemoteSession");		
		
	}
}
