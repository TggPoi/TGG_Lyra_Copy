// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameData.h"
#include "LyraAssetManager.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameData)



ULyraGameData::ULyraGameData()
{
	
}


const ULyraGameData& ULyraGameData::ULyraGameData::Get()
{
	//调用资产管理的方法即可
	return ULyraAssetManager::Get().GetGameData();
}
