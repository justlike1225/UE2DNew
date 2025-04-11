// Private/AI/BST/BTService_UpdateCombatStatus.cpp

#include "AI/BST/BTService_UpdateCombatStatus.h" // 确认路径正确
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Enemies/EvilCreature.h" // 仍然依赖这个具体类
#include "Components/HealthComponent.h"
#include "Components/EnemyMeleeAttackComponent.h"
#include "Components/TeleportComponent.h"
#include "Engine/Engine.h" // 如果需要日志或屏幕输出

// 构造函数
UBTService_UpdateCombatStatus::UBTService_UpdateCombatStatus()
{
    NodeName = "Update Combat Status";

  
}

// TickNode 函数
void UBTService_UpdateCombatStatus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds); // 调用父类 Tick

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    AAIController* AIController = OwnerComp.GetAIOwner();

    if (!BlackboardComp || !AIController)
    {
        return; // 基础检查
    }

    // 维持 Cast 到 EvilCreature，如果需要通用性再修改
    AEvilCreature* ControlledPawn = Cast<AEvilCreature>(AIController->GetPawn());
    if (!ControlledPawn)
    {
        // 考虑添加日志：UE_LOG(LogTemp, Warning, TEXT("BTService_UpdateCombatStatus: Controlled Pawn is not AEvilCreature!"));
        return;
    }

    // 获取组件
    UHealthComponent* HealthComp = ControlledPawn->GetHealthComponent();
    UEnemyMeleeAttackComponent* MeleeComp = ControlledPawn->GetMeleeAttackComponent();
    UTeleportComponent* TeleportComp = ControlledPawn->GetTeleportComponent();

    // 更新 DistanceToTarget
    // 检查 TargetActorKey 的名字是否有效
    if (!TargetActorKey.SelectedKeyName.IsNone())
    {
        AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
        // 检查 DistanceToTargetKey 的名字是否有效
        if (!DistanceToTargetKey.SelectedKeyName.IsNone())
        {
            if (TargetActor)
            {
                float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
                BlackboardComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);
            }
            else
            {
                BlackboardComp->ClearValue(DistanceToTargetKey.SelectedKeyName);
            }
        }
        else
        {
             // 可选日志：UE_LOG(LogTemp, Warning, TEXT("BTService_UpdateCombatStatus: DistanceToTargetKey name is None!"));
        }
    }
     else
    {
         // 可选日志：UE_LOG(LogTemp, Warning, TEXT("BTService_UpdateCombatStatus: TargetActorKey name is None!"));
         // 如果 TargetActorKey 未设置，也清理距离键
         if (!DistanceToTargetKey.SelectedKeyName.IsNone()) {
              BlackboardComp->ClearValue(DistanceToTargetKey.SelectedKeyName);
         }
    }


    // 更新 CanMeleeAttack
    // 检查 CanMeleeAttackKey 的名字是否有效
    if (!CanMeleeAttackKey.SelectedKeyName.IsNone())
    {
        bool bCanMelee = MeleeComp ? MeleeComp->CanAttack() : false;
        BlackboardComp->SetValueAsBool(CanMeleeAttackKey.SelectedKeyName, bCanMelee);
    }
     else
    {
         // 可选日志：UE_LOG(LogTemp, Warning, TEXT("BTService_UpdateCombatStatus: CanMeleeAttackKey name is None!"));
    }

    // 更新 CanTeleport
    // 检查 CanTeleportKey 的名字是否有效
    if (!CanTeleportKey.SelectedKeyName.IsNone())
    {
        bool bCanTeleport = TeleportComp ? TeleportComp->CanTeleport() : false;
        BlackboardComp->SetValueAsBool(CanTeleportKey.SelectedKeyName, bCanTeleport);
    }
     else
    {
         // 可选日志：UE_LOG(LogTemp, Warning, TEXT("BTService_UpdateCombatStatus: CanTeleportKey name is None!"));
    }

    // 更新 IsHealthLow
    // 检查 IsHealthLowKey 的名字是否有效
    if (!IsHealthLowKey.SelectedKeyName.IsNone())
    {
        if (HealthComp)
        {
            float CurrentHealth = HealthComp->GetCurrentHealth();
            float MaxHealth = HealthComp->GetMaxHealth();
            // 使用 .h 文件中定义的 UPROPERTY
            bool bIsLow = (MaxHealth > 0 && (CurrentHealth / MaxHealth) <= HealthLowThreshold);
            BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, bIsLow);
        }
        else
        {
            BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, false); // 没有生命组件则认为不是低血量
        }
    }
     else
    {
        // 可选日志：UE_LOG(LogTemp, Warning, TEXT("BTService_UpdateCombatStatus: IsHealthLowKey name is None!"));
    }
}

