// My2DGameDesign/Private/AI/Tasks/BTTask_ExecuteMeleeAttack.cpp
#include "AI/Tasks/BTTask_ExecuteMeleeAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Enemies/EvilCreature.h" // 需要 Pawn 类型
#include "Components/EnemyMeleeAttackComponent.h" // 需要组件

UBTTask_ExecuteMeleeAttack::UBTTask_ExecuteMeleeAttack()
{
	NodeName = "Execute Melee Attack";
	bNotifyTick = true; // **重要:** 我们需要在 TickTask 中检查状态，所以必须设为 true
    bNotifyTaskFinished = true; // 如果需要处理 AbortTask 里的异步完成
}

// 返回节点内存的大小
uint16 UBTTask_ExecuteMeleeAttack::GetInstanceMemorySize() const
{
	return sizeof(FBTExecuteMeleeAttackMemory);
}

// 初始化内存
void UBTTask_ExecuteMeleeAttack::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    // 获取内存块并初始化
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
    MyMemory->bIsExecutingAttack = false;
    MyMemory->MeleeCompPtr = nullptr;
}

// 清理内存 (可选，但好习惯)
void UBTTask_ExecuteMeleeAttack::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
    FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
    MyMemory->bIsExecutingAttack = false;
    MyMemory->MeleeCompPtr = nullptr;
}


EBTNodeResult::Type UBTTask_ExecuteMeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 获取节点内存
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
    MyMemory->bIsExecutingAttack = false; // 重置状态

	// 获取近战组件
	UEnemyMeleeAttackComponent* MeleeComp = GetMeleeComponent(OwnerComp);
	if (!MeleeComp)
	{
		return EBTNodeResult::Failed; // 找不到组件则失败
	}

	// (可选) 获取目标 Actor
	AActor* TargetActor = nullptr;
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp && TargetActorKey.IsSet()) // 检查 Key 是否在编辑器中被设置
	{
		TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	}

	// 尝试执行攻击
	if (MeleeComp->ExecuteAttack(TargetActor))
	{
        // 攻击成功开始
		MyMemory->bIsExecutingAttack = true; // 标记为正在执行
        MyMemory->MeleeCompPtr = MeleeComp; // 缓存组件指针方便 Tick 使用
		return EBTNodeResult::InProgress; // 任务进入 InProgress 状态，等待 TickTask 检查完成
	}
	else
	{
        // 攻击未能开始 (比如在冷却中)
		return EBTNodeResult::Failed;
	}
}

void UBTTask_ExecuteMeleeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);

	// 只有在 ExecuteTask 成功启动攻击后才检查
	if (MyMemory->bIsExecutingAttack)
	{
        UEnemyMeleeAttackComponent* MeleeComp = MyMemory->MeleeCompPtr.Get(); // 从弱指针获取

        // 检查组件是否仍然有效，并且攻击状态是否已结束
		if (MeleeComp && !MeleeComp->IsAttacking()) // **重要:** 检查 IsAttacking() 状态
		{
            // 攻击动画或流程已完成
			MyMemory->bIsExecutingAttack = false; // 重置内存状态
            MyMemory->MeleeCompPtr = nullptr;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded); // 标记任务成功完成
		}
        else if (!MeleeComp) // 如果组件中途失效了
        {
            MyMemory->bIsExecutingAttack = false;
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        }
        // else: 组件仍然有效且 IsAttacking() 仍为 true，继续等待下一个 Tick
	}
    else // 如果 bIsExecutingAttack 是 false，但 TickTask 被调用了，说明可能出错了，直接结束
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
}

EBTNodeResult::Type UBTTask_ExecuteMeleeAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);

	if (MyMemory->bIsExecutingAttack)
	{
        UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteMeleeAttack Aborted while executing."));
        // 如果近战组件支持中断，可以在这里调用中断函数
        UEnemyMeleeAttackComponent* MeleeComp = MyMemory->MeleeCompPtr.Get();
        // if (MeleeComp) { MeleeComp->InterruptAttack(); } // 假设有 InterruptAttack 函数

		MyMemory->bIsExecutingAttack = false; // 重置状态
        MyMemory->MeleeCompPtr = nullptr;
	}

    // 调用 FinishLatentAbort 来正确处理 latent task 的中断
	FinishLatentAbort(OwnerComp);
	return EBTNodeResult::Aborted;
}


// 辅助函数
UEnemyMeleeAttackComponent* UBTTask_ExecuteMeleeAttack::GetMeleeComponent(UBehaviorTreeComponent& OwnerComp) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AEvilCreature* ControlledPawn = Cast<AEvilCreature>(AIController->GetPawn());
		if (ControlledPawn)
		{
			return ControlledPawn->GetMeleeAttackComponent();
		}
	}
	return nullptr;
}