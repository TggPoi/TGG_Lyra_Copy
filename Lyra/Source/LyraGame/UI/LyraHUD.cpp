// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraHUD.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Async/TaskGraphInterfaces.h"
#include "Components/GameFrameworkComponentManager.h"
#include "UObject/UObjectIterator.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraHUD)


class AActor;
class UWorld;


//////////////////////////////////////////////////////////////////////
// ALyraHUD

ALyraHUD::ALyraHUD(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ALyraHUD::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ALyraHUD::BeginPlay()
{
	//在BeginPlay执行后还可以多一个初始化步骤 GameActorReady，拓展初始化流程
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void ALyraHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	
	Super::EndPlay(EndPlayReason);
}

void ALyraHUD::GetDebugActorList(TArray<AActor*>& InOutList)
{
	UWorld* World = GetWorld();
	
	Super::GetDebugActorList(InOutList);

	
	// Add all actors with an ability system component.
	// 将所有带有能力系统组件的角色添加进来。
	for (TObjectIterator<UAbilitySystemComponent> It; It; ++It)
	{
		if (UAbilitySystemComponent* ASC = *It)
		{
			// 	RF_ClassDefaultObject		=0x00000010,	
			//	This object is used as the default template for all instances of a class. One object is created for each class
			//  不能是CDO
			//	RF_ArchetypeObject			=0x00000020,
			//  This object can be used as a template for instancing objects. This is set on all types of object templates
			//	不能是模板
			if (!ASC->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
			{

				//OwnerActor is the actor that logically owns this component.
				//AvatarActor is what physical actor in the world we are acting on. Usually a Pawn but it could be a Tower, Building, Turret, etc, may be the same as Owner
				//“OwnerActor”指的是在逻辑上拥有此组件的“角色”。
				//“AvatarActor”则是我们在现实世界中所扮演角色的“实体角色”。通常指的是“兵”类角色，但也可能是“塔”、“建筑”、“炮塔”等，可能与“Owner”相同。
				AActor* AvatarActor = ASC->GetAvatarActor();
				AActor* OwnerActor = ASC->GetOwnerActor();

				//是否是替身持有ASC组件
				//是否是逻辑所有者持有ASC

				if (AvatarActor && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AvatarActor))
				{
					AddActorToDebugList(AvatarActor, InOutList, World);
				}
				else if (OwnerActor && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor))
				{
					AddActorToDebugList(OwnerActor, InOutList, World);
				}
				
			}
			


			
		}
		
	}

	
}
