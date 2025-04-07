// My2DGameDesign/Private/Components/AfterImagePoolComponent.cpp

#include "Components/AfterImagePoolComponent.h"
#include "AfterImageActor.h" // 包含Actor头文件
#include "Engine/World.h"    // 需要 GetWorld()

UAfterImagePoolComponent::UAfterImagePoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    CurrentTotalPooledActors = 0;
}

void UAfterImagePoolComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!PooledActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AfterImagePoolComponent: PooledActorClass is not set!"));
		return;
	}

	// 预生成初始数量的Actor
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]() { // 延迟一帧执行，确保World初始化充分
        if (!GetWorld()) return; // 再次检查World
		for (int32 i = 0; i < InitialPoolSize; ++i)
		{
			AAfterImageActor* NewActor = TryGrowPool();
            if (NewActor)
            {
                // 新生成的Actor默认就是非激活状态，直接加入InactivePool
                InactivePool.Add(NewActor);
            }
            else
            {
                // 如果初始阶段就无法生成，可能MaxPoolSize设置太小，或者Spawn失败
                UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolComponent: Failed to spawn initial pool actor %d/%d."), i + 1, InitialPoolSize);
                break; // 不再尝试生成更多初始对象
            }
		}
        UE_LOG(LogTemp, Log, TEXT("AfterImagePoolComponent: Initialized with %d inactive actors. Total managed: %d"), InactivePool.Num(), CurrentTotalPooledActors);
    });
}

AAfterImageActor* UAfterImagePoolComponent::SpawnFromPool(
    UPaperFlipbook* FlipbookToCopy,
    UMaterialInterface* MaterialToUse,
    float LifeTime,
    const FTransform& SpriteTransform,
    FName OpacityParamName,
    float InitialOpacity,
    float FadeUpdateInterval)
{
	AAfterImageActor* ActorToActivate = nullptr;

	// 1. 尝试从非激活池中获取
	if (InactivePool.Num() > 0)
	{
		ActorToActivate = InactivePool.Pop(); // 从末尾取出一个，效率较高
        UE_LOG(LogTemp, Verbose, TEXT("AfterImagePoolComponent: Reusing actor from pool. Inactive left: %d"), InactivePool.Num());
	}
	// 2. 如果池空了，并且允许增长，尝试生成新的
	else if (bAllowPoolGrowth)
	{
        UE_LOG(LogTemp, Verbose, TEXT("AfterImagePoolComponent: Inactive pool empty, trying to grow pool..."));
		ActorToActivate = TryGrowPool();
        if(ActorToActivate)
        {
             UE_LOG(LogTemp, Log, TEXT("AfterImagePoolComponent: Pool grew. Total managed: %d"), CurrentTotalPooledActors);
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolComponent: Failed to grow pool (Max size reached or spawn failed)."));
        }
	}
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolComponent: Inactive pool empty and growth not allowed. Cannot spawn."));
    }


	// 3. 如果获取或生成成功，则激活Actor
	if (ActorToActivate)
	{
		ActorToActivate->Activate(
			FlipbookToCopy,
			MaterialToUse,
			LifeTime,
			SpriteTransform,
			OpacityParamName,
			InitialOpacity,
			FadeUpdateInterval,
			this // <--- 将自身指针传给Actor
		);
		return ActorToActivate;
	}

	// 4. 获取或生成失败
	return nullptr;
}



void UAfterImagePoolComponent::ReturnToPool(AAfterImageActor* ActorToReturn)
{
	if (!ActorToReturn)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAfterImagePoolComponent::ReturnToPool - Attempted to return a null actor."));
		return;
	}

	// 可以考虑在这里添加一个检查 Actor 是否已失活的逻辑 (例如检查 ActorTick 是否启用等)
	// if (!ActorToReturn->IsActorTickEnabled()) { return; } // 这是一个例子，你需要根据 Actor 的失活状态选择合适的检查

	UE_LOG(LogTemp, Log, TEXT("UAfterImagePoolComponent::ReturnToPool - Returning actor %s"), *ActorToReturn->GetName());

	// Deactivate the actor (sets visibility, collision off etc.)
	// ActorToReturn->Deactivate(); // <--- 核心修改：删除或注释掉这一行！！！

	// Add to inactive pool if not already there (你的代码中已经有这个检查了，很好！)
	if (!InactivePool.Contains(ActorToReturn))
	{
		InactivePool.Add(ActorToReturn);
		UE_LOG(LogTemp, Log, TEXT("UAfterImagePoolComponent::ReturnToPool - Actor %s added to inactive pool."), *ActorToReturn->GetName());
	}
	else
	{
		// Actor 已经在池中，记录警告
		UE_LOG(LogTemp, Warning, TEXT("UAfterImagePoolComponent::ReturnToPool - Actor %s was already in the inactive pool!"), *ActorToReturn->GetName());
	}
}

AAfterImageActor* UAfterImagePoolComponent::TryGrowPool()
{
    // 检查是否达到最大容量 (如果MaxPoolSize > 0)
	if (MaxPoolSize > 0 && CurrentTotalPooledActors >= MaxPoolSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolComponent: Max pool size (%d) reached. Cannot grow further."), MaxPoolSize);
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World || !PooledActorClass) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner(); // 池组件的拥有者作为新Actor的拥有者
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // 生成时忽略碰撞

	AAfterImageActor* NewActor = World->SpawnActor<AAfterImageActor>(PooledActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (NewActor)
	{
        CurrentTotalPooledActors++;
		AllManagedActors.Add(NewActor); // 加入总管理列表
		NewActor->Deactivate(); // 确保新生成的Actor初始是Deactivated状态
        UE_LOG(LogTemp, Verbose, TEXT("AfterImagePoolComponent: Successfully grew pool. Spawned '%s'. Total managed: %d"), *NewActor->GetName(), CurrentTotalPooledActors);
		return NewActor;
	}
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AfterImagePoolComponent: Failed to spawn a new actor of class '%s'."), *PooledActorClass->GetName());
    }
	return nullptr;
}


void UAfterImagePoolComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("AfterImagePoolComponent: EndPlay called. Destroying %d managed actors."), AllManagedActors.Num());

	// 清理池中的所有Actor
	for (AAfterImageActor* Actor : AllManagedActors)
	{
		if (Actor && !Actor->IsPendingKillPending()) // 检查Actor是否有效且未被标记销毁
		{
            Actor->SetLifeSpan(0.1f); // 或者直接 Destroy()，设置短暂生命周期更安全一点
            // Actor->Destroy();
		}
	}
	AllManagedActors.Empty();
	InactivePool.Empty();
    CurrentTotalPooledActors = 0;

	Super::EndPlay(EndPlayReason);
}