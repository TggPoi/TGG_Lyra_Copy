// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraExperienceDefinition.h"
#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraExperienceDefinition)

#define LOCTEXT_NAMESPACE "LyraSystem"

ULyraExperienceDefinition::ULyraExperienceDefinition()
{
	
}

#if WITH_EDITOR
EDataValidationResult ULyraExperienceDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const UGameFeatureAction* Action : Actions)
	{
		//遍历所有动作，如果有一个动作是无效的，那么整个体验包就是无效的
		if (Action)
		{
			EDataValidationResult ChildResult = Action->IsDataValid(Context);
			Result = CombineDataValidationResults(Result, ChildResult);
		}else //如果不存在，说明是有问题的
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in Actions"), FText::AsNumber(EntryIndex)));
		}
		++EntryIndex;
	}
	
	// 确保用户没有从本业务包的基类派生出新的类（这种情况是正常的且预期的，即在一个业务包中只能派生一次子类，不能两次）
	// Make sure users didn't subclass from a BP of this (it's fine and expected to subclass once in BP, just not twice)
	if (!GetClass()->IsNative())
	{
		const UClass* ParentClass = GetClass()->GetSuperClass();
		
		const UClass* FirstNativeParent = ParentClass;

		//一直遍历，直到找到第一个原生类
		while ((FirstNativeParent != nullptr) && !FirstNativeParent->IsNative())
		{
			//获取顶级父类
			FirstNativeParent = FirstNativeParent->GetSuperClass();
		}

		//确保当前类的顶级父类，和直接父类是同一个类！ 如果原始父类和父类不是同一个,证明发生了多次继承.这是不合理的.
		if (FirstNativeParent != ParentClass)
		{
			//格式化的固定写法
			Context.AddError(FText::Format(LOCTEXT("ExperienceInheritenceIsUnsupported", "Blueprint subclasses of Blueprint experiences is not currently supported (use composition via ActionSets instead). Parent class was {0} but should be {1}."), 
				FText::AsCultureInvariant(GetPathNameSafe(ParentClass)),
				FText::AsCultureInvariant(GetPathNameSafe(FirstNativeParent))
			));
			Result = EDataValidationResult::Invalid;
		}

	}
	
	return Result;
}


#endif


#if WITH_EDITORONLY_DATA
void ULyraExperienceDefinition::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
	
}

#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE