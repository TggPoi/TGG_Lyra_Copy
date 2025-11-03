// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
// 001 定义接口和方法即可.
#pragma once

#include "GenericTeamAgentInterface.h"
#include "UObject/Object.h"

#include "UObject/WeakObjectPtr.h"
#include "LyraTeamAgentInterface.generated.h"

#define UE_API LYRAGAME_API

//前向声明UE提供的模板类
template <typename InterfaceType> class TScriptInterface;

//队伍索引发生改变时触发
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLyraTeamIndexChangedDelegate, UObject*, ObjectChangingTeam, int32, OldTeamID, int32, NewTeamID);

/*
 * 将引擎提供的队伍ID FGenericTeamId转换为int32
 * 将int32转换为FGenericTeamId
 */
inline int32 GenericTeamIdToInteger(FGenericTeamId ID)
{
	return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : (int32)ID;
}

/*
 * 将int32转换为引擎提供的队伍ID FGenericTeamId
 * 将FGenericTeamId转换为int32
 */
inline FGenericTeamId IntegerToGenericTeamId(int32 ID)
{
	return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId((uint8)ID);
}

/** Interface for actors which can be associated with teams */
/** 用于表示可与团队相关联的演员的接口 */
/*
 * UGenericTeamAgentInterface 实现这个接口用于ai识别队友是 中立，敌人，友方，内部封装了判断是否同一队伍的函数
 */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class ULyraTeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ILyraTeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_IINTERFACE_BODY()

	// 获取队伍改变的代理
	/*
	 * 代理具体放在哪个对象实例身上取决于子类的实现
	 * lyra的思想是 队伍的信息是能够进行传递的，例如玩家身上有一把枪，枪射出的子弹是携带队伍信息的
	 * 此时代理应该放在玩家角色身上，子弹绑定在武器上，武器绑定在角色身上，这样子弹的队伍信息就能够传递到子弹的子弹上，而不需要每次射击时都给子弹重新赋值设置队伍信息
	 * 
	 */
	virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }


	// 广播队伍发生了改变
	static UE_API void ConditionalBroadcastTeamChanged(TScriptInterface<ILyraTeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);


	// 带有检查的获取队伍改变的代理
	FOnLyraTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
	{
		FOnLyraTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
		//避免子类没有实现这个接口，导致获取下级代理时，下级代理为空，导致队伍信息判断错误
		check(Result);
		return *Result;
	}
	
};









#undef UE_API


