// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LyraGame : ModuleRules
{
	public LyraGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				"LyraGame"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[] {
				
				// UE 基础类型和反射系统。
				"Core",
				"CoreUObject",
				
				// 在线功能基础支持（如登录、会话管理）。
				"CoreOnline",
				
				// 应用程序级功能（如窗口管理、输入事件）。
				"ApplicationCore",
				
				// 引擎核心（Actor、关卡、渲染等）。
				"Engine",
				
				// 物理系统基础支持。
				"PhysicsCore",
				
				// 游戏标签系统（用于技能、状态分类）。
				"GameplayTags",
				
				// 异步任务管理（如AI行为、技能流程）。
				"GameplayTasks",
				
				// 游戏技能系统（GA）的核心逻辑。
				"GameplayAbilities",
				
				// AI 行为树、导航系统。
				"AIModule",
				
				// 数据驱动注册表（动态加载配置数据）。
				"ModularGameplay",
				
				// 支持模块化设计的Actor类。
				"ModularGameplayActors",
				
				// 数据驱动注册表（动态加载配置数据）。
				"DataRegistry",
				
				// 优化的网络复制系统（大型地图同步）。
				"ReplicationGraph",
				
				// 游戏功能插件系统（动态加载DLC/模块）。
				"GameFeatures",
				
				// 重要度管理系统
				"SignificanceManager",
				
				// 热修复
				"Hotfix",
				
				// 通用加载屏幕管理。
				"CommonLoadingScreen",
				
				// 粒子特效系统。
				"Niagara",
				
				// 异步操作支持（混合到其他类中）。
				"AsyncMixin",
				
				// 复杂逻辑流程控制（如技能链、剧情分支）。
				"ControlFlows",
				
				// 提供动态访问和修改对象属性
				"PropertyPath"
				
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				
				// 输入设备支持。
				"InputCore",
				
				// UI 框架基础（非游戏内UI，如编辑器工具）。
				"Slate",
				"SlateCore",
				
				// 渲染核心
				"RenderCore",
				
				// 开发者设置
				"DeveloperSettings",
				
				// 增强型输入系统
				"EnhancedInput",
				
				// 网络核心
				"NetCore",
				
				// RHI模块 底层图形API抽象层：封装了 DirectX/Vulkan/Metal 等图形接口，提供跨平台的渲染指令。管理纹理、缓冲区、着色器等GPU资源。处理多线程渲染命令提交。
				"RHI",
				
				// 项目管理（插件、游戏路径等）
				"Projects",
				
				// 自动化测试框架
				"Gauntlet",
				
				// 游戏内UI（用户界面控件）。
				"UMG",
				
				// 通用UI组件（如菜单、HUD）。
				"CommonUI",
				
				// 跨平台输入统一管理。
				"CommonInput",
				
				// 游戏设置
				"GameSettings",
				
				// 通用游戏逻辑（如玩家管理、游戏模式）。
				"CommonGame",
				
				// 用户账户与权限系统。
				"CommonUser",
				
				// 字幕
				"GameSubtitles",
				
				// 游戏内消息总线（事件广播）。
				"GameplayMessageRuntime",
				
				// 高级音频混合与控制。
				"AudioMixer",
				
				// 网络回放功能（录制与播放游戏回放）。
				"NetworkReplayStreaming",
				
				// UI接入点
				"UIExtension",
				
				// 客户端自动化行为控制
				"ClientPilot",
				
				// 音频动态调节（根据游戏状态改变音效）。
				"AudioModulation",
				
				// 引擎设置
				"EngineSettings",
				
				// 加密网络通信支持（安全传输层）。
				"DTLSHandlerComponent",
				
				// JSON 数据解析与序列化。
				"Json",
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
			}
		);

		// Generate compile errors if using DrawDebug functions in test/shipping builds.
		// 若在测试版或正式版中使用 DrawDebug 函数，则会生成编译错误。
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

		// Basic setup for External RPC Framework.
		// Functionality within framework will be stripped in shipping to remove vulnerabilities.
		// 外部 RPC 框架的基本设置。
		// 在正式发布版本中，框架内的功能将被剔除，以消除潜在的安全漏洞。
		
		PrivateDependencyModuleNames.Add("ExternalRpcRegistry");
		PrivateDependencyModuleNames.Add("HTTPServer"); // Dependency for ExternalRpcRegistry
		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			PublicDefinitions.Add("WITH_RPC_REGISTRY=0");
			PublicDefinitions.Add("WITH_HTTPSERVER_LISTENERS=0");
		}
		else
		{
			PublicDefinitions.Add("WITH_RPC_REGISTRY=1");
			PublicDefinitions.Add("WITH_HTTPSERVER_LISTENERS=1");
		}
		// 为游戏调试器支持设置此模块
		SetupGameplayDebuggerSupport(Target);
		// 根据 UEBuildConfiguration 中的设置，为 Iris 支持设置此模块。
		SetupIrisSupport(Target);
	}
}
