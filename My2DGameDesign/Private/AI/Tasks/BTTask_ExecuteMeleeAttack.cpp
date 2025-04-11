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


EBTNodeResult::Type UBTTask_ExecuteMeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
	MyMemory->bIsExecutingAttack = false;


	UEnemyMeleeAttackComponent* MeleeComp = GetMeleeComponent(OwnerComp);
	if (!MeleeComp)
	{
		return EBTNodeResult::Failed;
	}


	AActor* TargetActor = nullptr;
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp && TargetActorKey.IsSet())
	{
		TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	}


	if (MeleeComp->ExecuteAttack(TargetActor))
	{
		MyMemory->bIsExecutingAttack = true;
		MyMemory->MeleeCompPtr = MeleeComp;
		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Failed;
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
