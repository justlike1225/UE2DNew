// My2DGameDesign/Private/AI/Tasks/BTTask_ExecuteTeleport.cpp
#include "AI/Tasks/BTTask_ExecuteTeleport.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Enemies/EvilCreature.h"
#include "Components/TeleportComponent.h"
#include "NavigationSystem.h" // 需要包含导航系统头文件

UBTTask_ExecuteTeleport::UBTTask_ExecuteTeleport()
{
	NodeName = "Execute Teleport";
	bNotifyTick = true; // 需要 Tick 来检查完成状态
    bNotifyTaskFinished = true; // 需要处理 Abort

    // 指定任务需要访问的黑板键 (可选，但有助于编辑器验证)
    BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ExecuteTeleport, SelfActorKey), AActor::StaticClass());
    // 根据模式条件性地要求 TargetActorKey
    // 注意：UBTTask_BlackboardBase 的 BlackboardKey 主要用于单个目标键，这里我们访问多个键，所以不直接设置它
}

uint16 UBTTask_ExecuteTeleport::GetInstanceMemorySize() const
{
    return sizeof(FBTExecuteTeleportMemory);
}

void UBTTask_ExecuteTeleport::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
    MyMemory->bIsExecutingTeleport = false;
    MyMemory->TeleportCompPtr = nullptr;
}

void UBTTask_ExecuteTeleport::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
    FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
    MyMemory->bIsExecutingTeleport = false;
    MyMemory->TeleportCompPtr = nullptr;
}


EBTNodeResult::Type UBTTask_ExecuteTeleport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
    MyMemory->bIsExecutingTeleport = false; // 重置

	UTeleportComponent* TeleportComp = GetTeleportComponent(OwnerComp);
	if (!TeleportComp || !TeleportComp->CanTeleport()) // 检查组件和冷却状态
	{
		return EBTNodeResult::Failed;
	}

	FVector TargetLocation;
	if (!CalculateTargetLocation(OwnerComp, TargetLocation)) // 计算目标点
	{
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteTeleport: Failed to calculate valid teleport location."));
		return EBTNodeResult::Failed;
	}

	// 执行传送
	if (TeleportComp->ExecuteTeleport(TargetLocation))
	{
		MyMemory->bIsExecutingTeleport = true;
        MyMemory->TeleportCompPtr = TeleportComp;
		return EBTNodeResult::InProgress; // 等待传送完成
	}
	else
	{
		return EBTNodeResult::Failed; // 未能开始传送
	}
}

void UBTTask_ExecuteTeleport::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);

	if (MyMemory->bIsExecutingTeleport)
	{
        UTeleportComponent* TeleportComp = MyMemory->TeleportCompPtr.Get();
		// 检查传送是否已完成
		if (TeleportComp && !TeleportComp->IsTeleporting())
		{
            MyMemory->bIsExecutingTeleport = false;
            MyMemory->TeleportCompPtr = nullptr;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
        else if (!TeleportComp) // 组件失效
        {
            MyMemory->bIsExecutingTeleport = false;
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        }
        // else: 仍在传送中，继续等待
	}
    else // 不应发生，但作为保险
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
}

EBTNodeResult::Type UBTTask_ExecuteTeleport::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	if (MyMemory->bIsExecutingTeleport)
	{
        UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteTeleport Aborted while executing."));
        // 是否需要中断传送？取决于 TeleportComponent 是否支持
        UTeleportComponent* TeleportComp = MyMemory->TeleportCompPtr.Get();
        // if(TeleportComp) { TeleportComp->CancelTeleport(); } // 假设有 Cancel 函数

        MyMemory->bIsExecutingTeleport = false;
        MyMemory->TeleportCompPtr = nullptr;
	}
	FinishLatentAbort(OwnerComp);
	return EBTNodeResult::Aborted;
}

// 辅助函数：获取传送组件
UTeleportComponent* UBTTask_ExecuteTeleport::GetTeleportComponent(UBehaviorTreeComponent& OwnerComp) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AEvilCreature* ControlledPawn = Cast<AEvilCreature>(AIController->GetPawn());
		if (ControlledPawn)
		{
			return ControlledPawn->GetTeleportComponent();
		}
	}
	return nullptr;
}

// 辅助函数：计算目标位置 (修改后，强制 Y 轴一致)
bool UBTTask_ExecuteTeleport::CalculateTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation) const
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* SelfPawn = AIController ? AIController->GetPawn() : nullptr; // 获取受控制的 Pawn

    // 基本检查
    if (!BlackboardComp || !SelfPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("CalculateTargetLocation: Blackboard or SelfPawn is null."));
        return false;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Warning, TEXT("CalculateTargetLocation: NavigationSystem is null."));
        return false;
    }

    // **** 新增：获取并存储角色当前的 Y 坐标 ****
    const float OriginalY = SelfPawn->GetActorLocation().Y-0.01f;
    UE_LOG(LogTemp, Verbose, TEXT("CalculateTargetLocation: Character Original Y: %.2f"), OriginalY);

    FNavLocation ResultLocation; // 用于存储导航查询结果
    bool bFoundLocation = false;
    FVector Origin; // 理论上的传送中心点

    // --- 计算理论传送中心点 Origin (根据传送模式) ---
    if (TeleportMode == ETeleportMode::Offensive || TeleportMode == ETeleportMode::Defensive)
    {
        AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
        if (!TargetActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("CalculateTargetLocation: TargetActor is null for Offensive/Defensive mode."));
            return false; // 进攻和防御都需要目标
        }

        FVector SelfLocation = SelfPawn->GetActorLocation();
        FVector TargetLocation = TargetActor->GetActorLocation();
        FVector DirectionToTarget = (TargetLocation - SelfLocation).GetSafeNormal2D(); // 使用 2D 安全归一化 (忽略 Z)

        if (TeleportMode == ETeleportMode::Offensive)
        {
            // 尝试传送到目标身后一点的位置 (只考虑 XZ 平面)
             FVector TargetBackDirection = TargetActor->GetActorForwardVector().GetSafeNormal2D() * -1.0f; // 获取目标后方 2D 方向
             Origin = TargetLocation + TargetBackDirection * OffensiveOffsetDistance;
             // (可选) 将 Origin 的 Y 强制设为原始 Y，保证搜索中心在同一平面
             // Origin.Y = OriginalY;
        }
        else // Defensive
        {
            // 尝试传送到远离目标的方向 (只考虑 XZ 平面)
             Origin = SelfLocation + (DirectionToTarget * -1.0f) * DefensiveTeleportDistance;
             // (可选) 将 Origin 的 Y 强制设为原始 Y
             // Origin.Y = OriginalY;
        }
         UE_LOG(LogTemp, Verbose, TEXT("CalculateTargetLocation: Calculated Origin (Offensive/Defensive): %s"), *Origin.ToString());
    }
    else // Random
    {
        FVector SelfLocation = SelfPawn->GetActorLocation();
        Origin = SelfLocation; // 随机模式下，以自身为中心
        // (可选) 将 Origin 的 Y 强制设为原始 Y
        // Origin.Y = OriginalY;
         UE_LOG(LogTemp, Verbose, TEXT("CalculateTargetLocation: Using Origin (Random): %s"), *Origin.ToString());
    }

    // --- 在计算出的理论位置附近寻找一个导航网格上的可达点 ---
    // 注意：这里 Origin 理论上 Y 已经是 OriginalY，或者接近，但 GetRandomReachablePointInRadius 仍可能返回 Y 有偏差的点
    bFoundLocation = NavSys->GetRandomReachablePointInRadius(Origin, LocationFindRadius, ResultLocation);

    // --- 如果在理想 Origin 附近找不到，尝试备选方案 (可选) ---
    if (!bFoundLocation)
    {
         UE_LOG(LogTemp, Log, TEXT("CalculateTargetLocation: Failed to find location near Origin. Trying fallback..."));
        // 如果在理想位置附近找不到，可以尝试直接在目标或自身附近找随机点作为备选
         if (TeleportMode == ETeleportMode::Offensive)
         {
             AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
             if (TargetActor)
             {
                 bFoundLocation = NavSys->GetRandomReachablePointInRadius(TargetActor->GetActorLocation(), RandomTeleportRadius, ResultLocation);
             }
         }
         else // Defensive or Random
         {
              bFoundLocation = NavSys->GetRandomReachablePointInRadius(SelfPawn->GetActorLocation(), RandomTeleportRadius, ResultLocation);
         }
    }

    // --- 处理找到的位置 ---
    if (bFoundLocation)
    {
        UE_LOG(LogTemp, Log, TEXT("CalculateTargetLocation: Found reachable location by NavSys: %s"), *ResultLocation.Location.ToString());

        // **** 关键修改：强制将找到的位置的 Y 坐标设置为角色原始的 Y 坐标 ****
        ResultLocation.Location.Y = OriginalY;

        OutLocation = ResultLocation.Location; // 设置最终输出位置
        UE_LOG(LogTemp, Log, TEXT("CalculateTargetLocation: Final location after forcing Y: %s"), *OutLocation.ToString());
        return true;
    }
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("CalculateTargetLocation: Failed to find any reachable teleport location."));
    }

    return false; // 如果最终没找到位置
}