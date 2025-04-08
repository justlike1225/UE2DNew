// My2DGameDesign/Private/Subsystems/AfterImagePoolSubsystem.cpp
#include "MySubSystems/AfterImagePoolSubsystem.h"
#include "Actors/AfterImageActor.h" // 需要 Actor 类
#include "Engine/World.h"          // 需要 GetWorld()
#include "GameFramework/GameModeBase.h" // (如果需要从 GameMode 获取什么的话)
#include "TimerManager.h"        // 需要 TimerManager for NextTick

// --- UGameInstanceSubsystem 接口实现 ---

void UAfterImagePoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("AfterImagePoolSubsystem: Initializing..."));

    // 【重要配置】在这里设置你要池化的 Actor 类！
    // 你需要找到你的 AAfterImageActor 的路径。通常是 /Script/你的项目名.AfterImageActor
    // 或者直接使用 StaticClass()
     PooledActorClass = AAfterImageActor::StaticClass(); // 直接使用静态类引用更安全
     if (!PooledActorClass)
     {
         UE_LOG(LogTemp, Error, TEXT("AfterImagePoolSubsystem: PooledActorClass is not set correctly in C++!"));
         return;
     }

    // 延迟一帧执行预热，确保World有效
     FTimerHandle TempHandle;
     GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UAfterImagePoolSubsystem::PrewarmPool);
     // 注意：这里GetWorld()可能在某些GameInstance初始化阶段返回Null，延迟执行更安全
}

void UAfterImagePoolSubsystem::Deinitialize()
{
     UE_LOG(LogTemp, Log, TEXT("AfterImagePoolSubsystem: Deinitializing..."));
     CleanupPool(); // 清理池中所有 Actor
     Super::Deinitialize();
}


// --- 预热和清理函数 ---

void UAfterImagePoolSubsystem::PrewarmPool()
{
    if (!GetWorld() || !PooledActorClass) return; // 再次检查

    UE_LOG(LogTemp, Log, TEXT("AfterImagePoolSubsystem: Prewarming pool with %d actors..."), InitialPoolSize);
    for (int32 i = 0; i < InitialPoolSize; ++i)
    {
        AAfterImageActor* NewActor = TryGrowPool();
        if (NewActor)
        {
            InactivePool.Add(NewActor); // 新生成的直接加入非激活池
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolSubsystem: Failed to prewarm actor %d/%d."), i + 1, InitialPoolSize);
            break;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("AfterImagePoolSubsystem: Prewarming complete. Inactive: %d, Total: %d"), InactivePool.Num(), CurrentTotalPooledActors);
}

void UAfterImagePoolSubsystem::CleanupPool()
{
    UE_LOG(LogTemp, Log, TEXT("AfterImagePoolSubsystem: Cleaning up %d managed actors."), AllManagedActors.Num());
    // 使用迭代器或反向循环来安全地销毁和移除
    for (int32 i = AllManagedActors.Num() - 1; i >= 0; --i)
    {
         AAfterImageActor* Actor = AllManagedActors[i];
         if (Actor && !Actor->IsPendingKillPending())
         {
             Actor->SetLifeSpan(0.1f); // 设置短暂生命周期让引擎安全销毁
         }
    }
    AllManagedActors.Empty();
    InactivePool.Empty();
    CurrentTotalPooledActors = 0;
}


// --- 对象池核心逻辑 (基本从 Component 复制过来) ---

AAfterImageActor* UAfterImagePoolSubsystem::SpawnFromPool(
    UPaperFlipbook* FlipbookToCopy,
    UMaterialInterface* MaterialToUse,
    float LifeTime,
    const FTransform& SpriteTransform,
    FName OpacityParamName,
    float InitialOpacity,
    float FadeUpdateInterval)
{
    AAfterImageActor* ActorToActivate = nullptr;

    if (InactivePool.Num() > 0)
    {
        ActorToActivate = InactivePool.Pop();
         UE_LOG(LogTemp, Verbose, TEXT("AfterImagePoolSubsystem: Reusing actor. Inactive left: %d"), InactivePool.Num());
    }
    else if (bAllowPoolGrowth)
    {
         UE_LOG(LogTemp, Verbose, TEXT("AfterImagePoolSubsystem: Inactive pool empty, trying to grow pool..."));
        ActorToActivate = TryGrowPool();
        if(ActorToActivate)
        {
             UE_LOG(LogTemp, Log, TEXT("AfterImagePoolSubsystem: Pool grew. Total managed: %d"), CurrentTotalPooledActors);
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolSubsystem: Failed to grow pool."));
        }
    }
     else
     {
          UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolSubsystem: Inactive pool empty and growth not allowed."));
     }

    if (ActorToActivate)
    {// --- 添加日志 3 ---
        UE_LOG(LogTemp, Warning, TEXT("SpawnFromPool: ====> Calling Activate on Actor %p (%s)"), ActorToActivate, ActorToActivate ? *ActorToActivate->GetName() : TEXT("NULL"));
        ActorToActivate->Activate( // 调用 Actor 的 Activate
            FlipbookToCopy,
            MaterialToUse,
            LifeTime,
            SpriteTransform,
            OpacityParamName,
            InitialOpacity,
            FadeUpdateInterval,
            this // 把 Subsystem 自己的指针传给 Actor
        );
        return ActorToActivate;
    }

    return nullptr;
}

void UAfterImagePoolSubsystem::ReturnToPool(AAfterImageActor* ActorToReturn)
{
    if (!ActorToReturn)
    {
         UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolSubsystem::ReturnToPool - Null actor returned."));
        return;
    }

     UE_LOG(LogTemp, Verbose, TEXT("AfterImagePoolSubsystem: Returning actor %s to pool."), *ActorToReturn->GetName());

    // Actor 自身的 Deactivate 应该已经被调用了，这里只负责回收
    // 确保 Actor 处于非激活状态（可选，取决于 Actor 的 Deactivate 实现）
     if (ActorToReturn->IsActive()) // 假设 Actor 有 IsActive() 函数
     {
          UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolSubsystem::ReturnToPool - Actor %s is still active! Forcing Deactivate before pooling."), *ActorToReturn->GetName());
         // ActorToReturn->Deactivate(); // 这里不应该再调用 Deactivate，防止无限循环。应该由 Actor 自己调用 ReturnToPool
     }

     // 检查是否已在池中，避免重复添加
    if (!InactivePool.Contains(ActorToReturn))
    {
        InactivePool.Add(ActorToReturn);
        UE_LOG(LogTemp, Verbose, TEXT("AfterImagePoolSubsystem: Actor %s added to inactive pool. Count: %d"), *ActorToReturn->GetName(), InactivePool.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolSubsystem::ReturnToPool - Actor %s was already in the inactive pool!"), *ActorToReturn->GetName());
    }
}

AAfterImageActor* UAfterImagePoolSubsystem::TryGrowPool()
{
    if (MaxPoolSize > 0 && CurrentTotalPooledActors >= MaxPoolSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("AfterImagePoolSubsystem: Max pool size (%d) reached."), MaxPoolSize);
        return nullptr;
    }

    UWorld* World = GetWorld(); // Subsystem 可以通过 GetWorld() 获取 World
    if (!World || !PooledActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("AfterImagePoolSubsystem: Cannot grow pool - World or PooledActorClass invalid."));
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    // SpawnParams.Owner = ??? // Subsystem 通常没有 Owner，可以不设或设为 GameInstance?
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AAfterImageActor* NewActor = World->SpawnActor<AAfterImageActor>(PooledActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

    if (NewActor)
    {
        CurrentTotalPooledActors++;
        AllManagedActors.Add(NewActor);
        NewActor->Deactivate(); // 确保新生成的处于非激活状态
         UE_LOG(LogTemp, Verbose, TEXT("AfterImagePoolSubsystem: Spawned new actor '%s'. Total managed: %d"), *NewActor->GetName(), CurrentTotalPooledActors);
        return NewActor;
    }
     else
     {
         UE_LOG(LogTemp, Error, TEXT("AfterImagePoolSubsystem: Failed to spawn actor of class '%s'."), *PooledActorClass->GetName());
     }
    return nullptr;
}