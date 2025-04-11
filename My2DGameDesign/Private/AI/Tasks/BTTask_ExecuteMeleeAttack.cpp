#include "AI/Tasks/BTTask_ExecuteMeleeAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Enemies/EvilCreature.h"
#include "Components/EnemyMeleeAttackComponent.h"

UBTTask_ExecuteMeleeAttack::UBTTask_ExecuteMeleeAttack()
{
	NodeName = "Execute Melee Attack";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}


uint16 UBTTask_ExecuteMeleeAttack::GetInstanceMemorySize() const
{
	return sizeof(FBTExecuteMeleeAttackMemory);
}


void UBTTask_ExecuteMeleeAttack::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                  EBTMemoryInit::Type InitType) const
{
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
	MyMemory->bIsExecutingAttack = false;
	MyMemory->MeleeCompPtr = nullptr;
}


void UBTTask_ExecuteMeleeAttack::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                               EBTMemoryClear::Type CleanupType) const
{
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
	MyMemory->bIsExecutingAttack = false;
	MyMemory->MeleeCompPtr = nullptr;
}


// 执行任务的核心逻辑
EBTNodeResult::Type UBTTask_ExecuteMeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
	MyMemory->bIsExecutingAttack = false; // 重置状态
	MyMemory->MeleeCompPtr = nullptr;

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	// 获取 Pawn 并直接转换为 AEnemyCharacterBase
	AEnemyCharacterBase* ControlledPawn = AIController ? Cast<AEnemyCharacterBase>(AIController->GetPawn()) : nullptr;

	// 基础检查
	if (!AIController || !BlackboardComp || !ControlledPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteMeleeAttack: Missing AIController, Blackboard, or Pawn!"));
		return EBTNodeResult::Failed;
	}
	UE_LOG(LogTemp, Log, TEXT("--- BTTask_ExecuteMeleeAttack: Task Started for %s ---"),
	       *ControlledPawn->GetName()); // <<< LOG 1: 任务开始
	
	

	AActor* TargetActor =Cast<AActor> (BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    //  输出 TargetActor 的值
	GEngine  ->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("BTTask_ExecuteMeleeAttack: TargetActor: %s"), *GetNameSafe(TargetActor))); // 调试信息
	if (TargetActorKey.IsSet()) // 检查 Key 是否在编辑器中设置了
	{
		TargetActor =Cast<AActor> (BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_ExecuteMeleeAttack: TargetActorKey is not set!"));
	}
   

	// 获取近战攻击组件
	UEnemyMeleeAttackComponent* MeleeComp = GetMeleeComponent(OwnerComp);
	if (!MeleeComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteMeleeAttack: Failed to get MeleeComponent on %s!"),
		       *ControlledPawn->GetName());
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteMeleeAttack: Calling MeleeComp->ExecuteAttack...")); // <<< LOG 8: 准备调用攻击组件
	// 尝试执行攻击 (传入目标 Actor，组件内部会选择攻击索引)
	if (MeleeComp->ExecuteAttack(TargetActor))
	{
		UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteMeleeAttack: ExecuteAttack returned true. Task InProgress for %s."),
		       *ControlledPawn->GetName());
		MyMemory->bIsExecutingAttack = true;
		MyMemory->MeleeCompPtr = MeleeComp; // 存储组件指针用于 TickTask 检查
		return EBTNodeResult::InProgress; // 攻击开始，任务等待完成
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteMeleeAttack: MeleeComp->ExecuteAttack returned false for %s."),
		       *ControlledPawn->GetName());
		// 攻击未能启动 (可能在冷却中或组件无效)
		return EBTNodeResult::Failed;
	}
}

void UBTTask_ExecuteMeleeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);


	if (MyMemory->bIsExecutingAttack)
	{
		UEnemyMeleeAttackComponent* MeleeComp = MyMemory->MeleeCompPtr.Get();


		if (MeleeComp && !MeleeComp->IsAttacking())
		{
			MyMemory->bIsExecutingAttack = false;
			MyMemory->MeleeCompPtr = nullptr;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else if (!MeleeComp)
		{
			MyMemory->bIsExecutingAttack = false;
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

EBTNodeResult::Type UBTTask_ExecuteMeleeAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);

	if (MyMemory->bIsExecutingAttack)
	{
		UEnemyMeleeAttackComponent* MeleeComp = MyMemory->MeleeCompPtr.Get();


		MyMemory->bIsExecutingAttack = false;
		MyMemory->MeleeCompPtr = nullptr;
	}


	FinishLatentAbort(OwnerComp);
	return EBTNodeResult::Aborted;
}


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
