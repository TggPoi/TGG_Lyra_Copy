// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "GameFramework/HUD.h"

#include "LyraHUD.generated.h"

namespace EEndPlayReason { enum Type : int; }


class AActor;
class UObject;

/**
 * ALyraHUD
 *
 *  Note that you typically do not need to extend or modify this class, instead you would
 *  use an "Add Widget" action in your experience to add a HUD layout and widgets to it
 *
 *  请注意，您通常无需对这个类进行扩展或修改，而是可以通过在您的体验中使用“添加控件”操作来添加 HUD 布局和控件到其中。
 *  
 *  This class exists primarily for debug rendering
 *  这个类的主要用途是用于调试渲染。
 */
UCLASS(Config = Game)
class ALyraHUD : public AHUD
{
	GENERATED_BODY()


public:
	//构造函数 关闭Tick
	ALyraHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


protected:

	//~UObject interface
	virtual void PreInitializeComponents() override;
	//~End of UObject interface

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AHUD interface
	/**
	 * 获取“显示调试信息”所涉及的目标列表
	 * 该列表是根据已启用的“显示调试信息”标志的上下文情况进行构建的。
	 * 
	 */
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
	//~End of AHUD interface
	


};
