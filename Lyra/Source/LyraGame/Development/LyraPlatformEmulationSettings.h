// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "GameplayTagContainer.h"
#include "LyraPlatformEmulationSettings.generated.h"

struct FPropertyChangedEvent;
/**
 * Platform emulation settings
 * 平台模拟设置
 */
UCLASS(config=EditorPerProjectUserSettings, MinimalAPI)
class ULyraPlatformEmulationSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()
	
public:
	ULyraPlatformEmulationSettings();

	//~UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface

	// 获取假装的平台配置文件
	FName GetPretendBaseDeviceProfile() const;

	// 获取假装的平台名称
	FName GetPretendPlatformName() const;
	
private:
	// 添加的平台特征开启
	UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(Categories="Input,Platform.Trait"))
	FGameplayTagContainer AdditionalPlatformTraitsToEnable;

	// 添加的平台特征压制
	UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(Categories="Input,Platform.Trait"))
	FGameplayTagContainer AdditionalPlatformTraitsToSuppress;

	// 假装的平台名称
	UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(GetOptions=GetKnownPlatformIds))
	FName PretendPlatform;

	// The base device profile to pretend we are using when emulating device-specific device profiles applied from ULyraSettingsLocal
	// 在模拟来自“ULyraSettingsLocal”中的设备特定设备配置文件时所使用的基础设备配置文件。
	UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(GetOptions=GetKnownDeviceProfiles, EditCondition=bApplyDeviceProfilesInPIE))
	FName PretendBaseDeviceProfile;
	
	
	// Do we apply desktop-style frame rate settings in PIE?
	// (frame rate limits are an engine-wide setting so it's not always desirable to have enabled in the editor)
	// You may also want to disable the editor preference "Use Less CPU when in Background" if testing background frame rate limits
	// 在 PIE 中，我们是否采用桌面式的帧率设置？
	// （帧率限制是引擎级别的设置，所以在编辑器中并非总是需要将其启用）
	// 如果要测试后台的帧率限制，您可能还需要禁用编辑器的偏好设置“在后台使用较少的 CPU”
	UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(ConsoleVariable="Lyra.Settings.ApplyFrameRateSettingsInPIE"))
	bool bApplyFrameRateSettingsInPIE = false;

	// Do we apply front-end specific performance options in PIE?
	// Most engine performance/scalability settings they drive are global, so if one PIE window
	// is in the front-end and the other is in-game one will win and the other gets stuck with those settings
	// 在 PIE 中，我们是否应用前端特定的性能选项？
	// 大多数引擎的性能/可扩展性设置都是全局性的，所以如果一个 PIE 窗口在前端运行，而另一个在游戏内运行，那么前者会占优，而后者则会一直受这些设置的限制。
	// 例如在编辑器中启动了两个视口实例，在其中一个视口调整性能设置时，因为在编辑器环境下这两个视口实例属于同一个进程，因此可能修改的是同一个变量，导致另一个视口也受到影响
	UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(ConsoleVariable="Lyra.Settings.ApplyFrontEndPerformanceOptionsInPIE"))
	bool bApplyFrontEndPerformanceOptionsInPIE = false;

	
	// Should we apply experience/platform emulated device profiles in PIE?
	// 我们是否应在 PIE 中应用经验/平台模拟设备配置文件？
	UPROPERTY(EditAnywhere, config, Category=PlatformEmulation, meta=(InlineEditConditionToggle, ConsoleVariable="Lyra.Settings.ApplyDeviceProfilesInPIE"))
	bool bApplyDeviceProfilesInPIE = false;

#if WITH_EDITOR
public:
	
	// Called by the editor engine to let us pop reminder notifications when cheats are active
	// 由编辑引擎调用，以便在作弊功能激活时向我们发送提醒通知
	LYRAGAME_API void OnPlayInEditorStarted() const;
	
private:
	// The last pretend platform we applied
	// 我们所使用的最后一个模拟平台
	FName LastAppliedPretendPlatform;

private:
	// 应用设置 将Tag传递给CommonUI系统
	void ApplySettings();
	
	// 更改激活的平台名称
	void ChangeActivePretendPlatform(FName NewPlatformName);
	
#endif
	
public:
	//~UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
	virtual void PostInitProperties() override;
#endif
	//~End of UObject interface

	
private:
	// 获取已知平台的名称
	UFUNCTION()
	TArray<FName> GetKnownPlatformIds() const;

	// 获取已知的配置文件
	UFUNCTION()
	TArray<FName> GetKnownDeviceProfiles() const;

	
	// 找到合理的基础配置文件
	void PickReasonableBaseDeviceProfile();
	
};