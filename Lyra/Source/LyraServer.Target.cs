// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraServerTarget : TargetRules
{
	public LyraServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		
		ExtraModuleNames.AddRange(new string[] { "LyraGame" });
		
		LyraGameTarget.ApplySharedLyraTargetSettings(this);
		
		// Whether to turn on checks (asserts) for test/shipping builds.
		// 是否在测试版/发布版中开启检查（断言）功能。
		bUseChecksInShipping = true;
		
	}
}
