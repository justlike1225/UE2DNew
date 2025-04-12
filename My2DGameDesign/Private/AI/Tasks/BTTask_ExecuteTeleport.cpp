
#include "AI/Tasks/BTTask_ExecuteTeleport.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

#include "NavigationSystem.h"
#include "Interfaces/AI/Abilities/TeleportAbilityExecutor.h" 
#include "Interfaces/AI/Status/CombatStatusProvider.h"    

// 构造函数不变...
UBTTask_ExecuteTeleport::UBTTask_ExecuteTeleport()
{
	NodeName = "Execute Teleport";
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ExecuteTeleport, SelfActorKey), AActor::StaticClass());
  
}



uint16 UBTTask_ExecuteTeleport::GetInstanceMemorySize() const
{
	return sizeof(FBTExecuteTeleportMemory);
}

void UBTTask_ExecuteTeleport::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	MyMemory->bIsExecutingTeleport = false;
	
}

void UBTTask_ExecuteTeleport::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	MyMemory->bIsExecutingTeleport = false;

}



// --- 3. 修改 ExecuteTask ---
EBTNodeResult::Type UBTTask_ExecuteTeleport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	MyMemory->bIsExecutingTeleport = false; // 重置状态

    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    if (!ControlledPawn) return EBTNodeResult::Failed;
	
    ICombatStatusProvider* StatusProvider = Cast<ICombatStatusProvider>(ControlledPawn);
    if (StatusProvider && !ICombatStatusProvider::Execute_CanPerformTeleport(ControlledPawn))
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteTeleport: Pawn cannot teleport now (checked via StatusProvider)."));
        return EBTNodeResult::Failed; // 如果不能传送，则失败
    }


	// 计算目标位置 (逻辑不变)
	FVector TargetLocation;
	if (!CalculateTargetLocation(OwnerComp, TargetLocation))
	{
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteTeleport: Failed to calculate target location."));
		return EBTNodeResult::Failed;
	}

    // --- 关键修改：通过接口执行传送 ---
    ITeleportAbilityExecutor* AbilityExecutor = Cast<ITeleportAbilityExecutor>(ControlledPawn);
    if (!AbilityExecutor)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteTeleport: Controlled Pawn '%s' does not implement ITeleportAbilityExecutor interface!"), *ControlledPawn->GetName());
        return EBTNodeResult::Failed; // Pawn 不具备执行传送的能力
    }

    // 调用接口函数执行传送
    if (ITeleportAbilityExecutor::Execute_ExecuteTeleportToLocation(ControlledPawn, TargetLocation))
	{
        // 传送已成功启动
        UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteTeleport: Execute_ExecuteTeleportToLocation returned true. Task InProgress for %s."), *ControlledPawn->GetName());
		MyMemory->bIsExecutingTeleport = true;
		// MyMemory->TeleportCompPtr = TeleportComp; // <--- 移除缓存组件指针
		return EBTNodeResult::InProgress; // 等待传送完成
	}
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteTeleport: Execute_ExecuteTeleportToLocation returned false for %s."), *ControlledPawn->GetName());
        return EBTNodeResult::Failed; // 启动失败
    }
}

// --- 4. 修改 TickTask ---
void UBTTask_ExecuteTeleport::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);

	if (MyMemory->bIsExecutingTeleport) // 检查任务是否仍在执行状态
	{
        AAIController* AIController = OwnerComp.GetAIOwner();
        APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;

        if (!ControlledPawn)
        {
             UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteTeleport::TickTask: Lost Controlled Pawn!"));
             MyMemory->bIsExecutingTeleport = false;
             FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
             return;
        }

      

        ICombatStatusProvider* StatusProvider = Cast<ICombatStatusProvider>(ControlledPawn);
        if (!StatusProvider)
        {
             UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteTeleport::TickTask: Controlled Pawn '%s' does not implement ICombatStatusProvider! Cannot check teleport status."), *ControlledPawn->GetName());
             MyMemory->bIsExecutingTeleport = false;
             FinishLatentTask(OwnerComp, EBTNodeResult::Failed); // 无法检查状态，失败
             return;
        }

        // 调用接口查询是否仍在传送
		if (!ICombatStatusProvider::Execute_IsPerformingTeleport(ControlledPawn)) // 如果传送结束
		{
            UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteTeleport::TickTask: IsPerformingTeleport is false. Task Succeeded for %s."), *ControlledPawn->GetName());
			MyMemory->bIsExecutingTeleport = false; // 重置状态
			// MyMemory->TeleportCompPtr = nullptr; // <--- 移除
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded); // 任务成功完成
		}
       
	}
	else 
	{
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteTeleport::TickTask: Called while not executing teleport?"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

// --- 5. 修改 AbortTask ---
EBTNodeResult::Type UBTTask_ExecuteTeleport::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	if (MyMemory->bIsExecutingTeleport)
	{
      
		MyMemory->bIsExecutingTeleport = false;
		
	}

    UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteTeleport: Task Aborted."));
	FinishLatentAbort(OwnerComp);
	return EBTNodeResult::Aborted;
}



bool UBTTask_ExecuteTeleport::CalculateTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation) const
{
   
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* SelfPawn = AIController ? AIController->GetPawn() : nullptr;

	if (!BlackboardComp || !SelfPawn) return false;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return false;

	const float OriginalY = SelfPawn->GetActorLocation().Y; // 修正：保持 Y 轴不变
	FNavLocation ResultLocation;
	bool bFoundLocation = false;
	FVector Origin = FVector::ZeroVector; // 初始化 Origin
    AActor* TargetActor = nullptr; // 初始化 TargetActor

    // 获取目标 Actor (如果需要)
	if (TeleportMode == ETeleportMode::Offensive || TeleportMode == ETeleportMode::Defensive)
	{
       
		    TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
   
		if (!TargetActor) return false; // 如果模式需要目标但目标无效，则失败
	}


    // --- 根据模式确定导航查询的中心点 Origin ---
	if (TeleportMode == ETeleportMode::Offensive)
	{
        // ... (进攻模式计算 Origin 的逻辑) ...
        FVector TargetLocation = TargetActor->GetActorLocation();
        // 尝试传送到目标背后一点的位置
        FVector TargetBackDirection = TargetActor->GetActorForwardVector().GetSafeNormal2D() * -1.0f;
        Origin = TargetLocation + TargetBackDirection * OffensiveOffsetDistance;
	}
	else if (TeleportMode == ETeleportMode::Defensive)
	{
        // ... (防御模式计算 Origin 的逻辑) ...
        FVector SelfLocation = SelfPawn->GetActorLocation();
        FVector TargetLocation = TargetActor->GetActorLocation();
		FVector DirectionToTarget = (TargetLocation - SelfLocation).GetSafeNormal2D();
        // 尝试传送到远离目标的方向
        Origin = SelfLocation + (DirectionToTarget * -1.0f) * DefensiveTeleportDistance;
	}
	else // Random Mode
	{
        // ... (随机模式计算 Origin 的逻辑) ...
		Origin = SelfPawn->GetActorLocation(); // 以自身为中心
	}


    // --- 执行导航查询 ---
	// 尝试在 Origin 附近找到一个可达点
	bFoundLocation = NavSys->GetRandomReachablePointInRadius(Origin, LocationFindRadius, ResultLocation);

	// 如果第一次查询失败，根据模式尝试备用查询
	if (!bFoundLocation)
	{
        float FallbackRadius = RandomTeleportRadius; // 备用半径
        FVector FallbackOrigin = FVector::ZeroVector;

        if (TeleportMode == ETeleportMode::Offensive && TargetActor)
        {
            FallbackOrigin = TargetActor->GetActorLocation(); // 尝试在目标周围找点
        }
        else // Defensive or Random, or Offensive failed to get TargetActor
        {
            FallbackOrigin = SelfPawn->GetActorLocation(); // 尝试在自己周围更大范围找点
        }
        UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteTeleport: Initial location find failed. Trying fallback around %s with radius %.1f."), *FallbackOrigin.ToString(), FallbackRadius);
		bFoundLocation = NavSys->GetRandomReachablePointInRadius(FallbackOrigin, FallbackRadius, ResultLocation);
	}


    // --- 处理结果 ---
	if (bFoundLocation)
	{
		ResultLocation.Location.Y = OriginalY; // 强制 Y 轴不变 (2D 约束)
		OutLocation = ResultLocation.Location;
        UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteTeleport: Found valid teleport location: %s"), *OutLocation.ToString());
		return true;
	}
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteTeleport: Failed to find any valid teleport location after fallback."));
    }

	return false; // 所有尝试都失败了
}