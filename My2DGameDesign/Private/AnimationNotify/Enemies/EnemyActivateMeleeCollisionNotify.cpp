
#include "AnimationNotify/Enemies/EnemyActivateMeleeCollisionNotify.h" // 包含自身头文件
#include "PaperZDAnimInstance.h"                     // PaperZD 动画实例基类
#include "GameFramework/Actor.h"                     // 需要获取 Actor
#include "Interfaces/AnimationEvents/EnemyAnimationEventHandler.h"


// 重写的接收通知函数
void UEnemyActivateMeleeCollisionNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
    // 1. 检查动画实例是否有效
    if (!OwningInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyActivateMeleeCollisionNotify: OwningInstance is null."));
        return;
    }

    // 2. 获取拥有此动画实例的 Actor
    AActor* OwnerActor = OwningInstance->GetOwningActor();
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyActivateMeleeCollisionNotify: OwnerActor is null."));
        return;
    }

   
    if (OwnerActor->GetClass()->ImplementsInterface(UEnemyAnimationEventHandler::StaticClass()))
    {
      
        IEnemyAnimationEventHandler::Execute_HandleAnim_ActivateMeleeCollision(OwnerActor, ShapeIdentifier, Duration);
        UE_LOG(LogTemp, Verbose, TEXT("EnemyActivateMeleeCollisionNotify: Called HandleAnim_ActivateMeleeCollision on %s"), *OwnerActor->GetName());
    }
    else
    {
        // 如果 Actor 没有实现接口，发出警告，因为通知无法被处理
        UE_LOG(LogTemp, Warning, TEXT("EnemyActivateMeleeCollisionNotify: Owner Actor %s does not implement IEnemyAnimationEventHandler! Notify ignored."), *OwnerActor->GetName());
    }
    
}