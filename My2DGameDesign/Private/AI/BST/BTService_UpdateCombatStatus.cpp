// Private/AI/BST/BTService_UpdateCombatStatus.cpp

#include "AI/BST/BTService_UpdateCombatStatus.h" // 确认路径正确
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/HealthComponent.h"
#include "Engine/Engine.h" // 如果需要日志或屏幕输出
#include "Interfaces/AI/Status/CombatStatusProvider.h"

// 构造函数
UBTService_UpdateCombatStatus::UBTService_UpdateCombatStatus()
{
    NodeName = "Update Combat Status";

  
}


// TickNode 函数
void UBTService_UpdateCombatStatus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    AAIController* AIController = OwnerComp.GetAIOwner();

    if (!BlackboardComp || !AIController)
    {
        return;
    }

    // --- 2. 获取 Pawn，不再 Cast 到 EvilCreature ---
    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        // 清理所有可能的状态值，因为没有 Pawn
        if (!CanMeleeAttackKey.SelectedKeyName.IsNone()) BlackboardComp->SetValueAsBool(CanMeleeAttackKey.SelectedKeyName, false);
        if (!CanTeleportKey.SelectedKeyName.IsNone()) BlackboardComp->SetValueAsBool(CanTeleportKey.SelectedKeyName, false);
        if (!IsHealthLowKey.SelectedKeyName.IsNone()) BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, false);
        if (!DistanceToTargetKey.SelectedKeyName.IsNone()) BlackboardComp->ClearValue(DistanceToTargetKey.SelectedKeyName);
        return;
    }

    // --- 更新 DistanceToTarget (逻辑不变) ---
    if (!TargetActorKey.SelectedKeyName.IsNone() && !DistanceToTargetKey.SelectedKeyName.IsNone())
    {
        AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
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
    // --- 距离更新结束 ---


    // --- 3. 通过接口获取能力状态 ---
    bool bCanMelee = false;
    bool bCanTeleport = false;
    // 尝试将 Pawn 转换为 CombatStatusProvider 接口
    ICombatStatusProvider* StatusProvider = Cast<ICombatStatusProvider>(ControlledPawn);
    if (StatusProvider) // 如果 Pawn 实现了这个接口
    {
        // 调用接口函数获取状态
        bCanMelee = ICombatStatusProvider::Execute_CanPerformMeleeAttack(ControlledPawn); // 注意 Execute_ 前缀
        bCanTeleport = ICombatStatusProvider::Execute_CanPerformTeleport(ControlledPawn); // 注意 Execute_ 前缀
    }
    // else: 如果没有实现接口，bCanMelee 和 bCanTeleport 会保持 false，这是合理的默认值

    // 更新黑板 (如果 Key 有效)
    if (!CanMeleeAttackKey.SelectedKeyName.IsNone())
    {
        BlackboardComp->SetValueAsBool(CanMeleeAttackKey.SelectedKeyName, bCanMelee);
    }
    if (!CanTeleportKey.SelectedKeyName.IsNone())
    {
        BlackboardComp->SetValueAsBool(CanTeleportKey.SelectedKeyName, bCanTeleport);
    }
    // --- 接口状态更新结束 ---


    // --- 4. 通过组件查询获取生命值状态 ---
    bool bIsLow = false;
    // 尝试查找 HealthComponent
    UHealthComponent* HealthComp = ControlledPawn->FindComponentByClass<UHealthComponent>();
    if (HealthComp) // 如果找到了生命组件
    {
        float CurrentHealth = HealthComp->GetCurrentHealth();
        float MaxHealth = HealthComp->GetMaxHealth();
        // 使用服务自身的阈值属性进行判断
        bIsLow = (MaxHealth > 0 && (CurrentHealth / MaxHealth) <= HealthLowThreshold);
    }
    // else: 如果没有 HealthComponent，bIsLow 会保持 false

    // 更新黑板 (如果 Key 有效)
    if (!IsHealthLowKey.SelectedKeyName.IsNone())
    {
        BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, bIsLow);
    }
    // --- 生命值状态更新结束 ---
}

