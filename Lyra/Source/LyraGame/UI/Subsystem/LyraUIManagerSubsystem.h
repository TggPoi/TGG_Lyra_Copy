// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "Containers/Ticker.h"
#include "GameUIManagerSubsystem.h"


#include "LyraUIManagerSubsystem.generated.h"

class FSubsystemCollectionBase;
class UObject;
UCLASS()
class ULyraUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:

	ULyraUIManagerSubsystem();

	// 绑定Tick
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 移除Tick
	virtual void Deinitialize() override;

private:
	// Tick
	bool Tick(float DeltaTime);
	
	// 更改UI布局的可实现
	void SyncRootLayoutVisibilityToShowHUD();
	
	FTSTicker::FDelegateHandle TickHandle;

};