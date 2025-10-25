// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "GameFramework/GameSession.h"

#include "LyraGameSession.generated.h"

class UObject;


UCLASS(Config = Game)
class ALyraGameSession : public AGameSession
{
	GENERATED_BODY()
public:

	ALyraGameSession(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:

	/**
	* 允许在线服务在命令行中通过 -auth_login 或 -auth_password 参数指定的情况下进行登录处理
	* @返回值：如果登录正在进行，则返回 true，否则返回 false
	*/
	/** 重写此方法以禁用默认行为 */
	virtual bool ProcessAutoLogin() override;

	
	/** 当比赛开始时的处理操作 */
	virtual void HandleMatchHasStarted() override;

	/** 当匹配完成时的处理操作 */
	virtual void HandleMatchHasEnded() override;
};