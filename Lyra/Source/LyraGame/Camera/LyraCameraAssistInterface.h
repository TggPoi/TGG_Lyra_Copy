// Copyright Epic Games, Inc. All Rights Reserved.
// Finished.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "LyraCameraAssistInterface.generated.h"


/**
 * 用于做相机侵入资产的接口.
 * 就是是否需要隐藏.
 */
UINTERFACE(BlueprintType)
class ULyraCameraAssistInterface : public UInterface
{
	GENERATED_BODY()
};

class ILyraCameraAssistInterface
{
	GENERATED_BODY()
	
	/**
	 * Get the list of actors that we're allowing the camera to penetrate. Useful in 3rd person cameras
	 * when you need the following camera to ignore things like the a collection of view targets, the pawn,
	 * a vehicle..etc.+
	 *
	 * 获取我们允许摄像机穿透的演员列表。在使用第三人称摄像机时非常有用。
	 * 当您需要以下摄像机忽略诸如一系列视点目标、角色、车辆等元素时，此功能非常有用。
	 * 
	 */
	virtual void GetIgnoredActorsForCameraPentration(TArray<const AActor*>& OutActorsAllowPenetration) const { }

	/**
	 * The target actor to prevent penetration on.  Normally, this is almost always the view target, which if
	 * unimplemented will remain true.  However, sometimes the view target, isn't the same as the root actor 
	 * you need to keep in frame.
	 *
	 * 需要防范入侵的目标对象。通常情况下，这几乎总是视图目标，即如果视图目标未实现，则该值始终为真。然而，有时视图目标与您需要保持在画面中的根对象并不相同。
	 * 
	 */
	virtual TOptional<AActor*> GetCameraPreventPenetrationTarget() const
	{
		return TOptional<AActor*>();
	}

	
	/** Called if the camera penetrates the focal target.  Useful if you want to hide the target actor when being overlapped. */
	/** 若相机穿透了焦点目标，则会调用此函数。若您希望在目标被覆盖时隐藏目标角色，则此功能非常有用。*/
	virtual void OnCameraPenetratingTarget() { }

	
	
	
};