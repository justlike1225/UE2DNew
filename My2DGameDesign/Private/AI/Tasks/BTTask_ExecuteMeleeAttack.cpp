
#include "AI/Tasks/BTTask_ExecuteMeleeAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/AI/Abilities/MeleeAbilityExecutor.h"
#include "Interfaces/AI/Status/CombatStatusProvider.h"
UBTTask_ExecuteMeleeAttack::UBTTask_ExecuteMeleeAttack()
{
    NodeName = "Execute Melee Attack (Interface)";
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

uint16 UBTTask_ExecuteMeleeAttack::GetInstanceMemorySize() const
{
    return sizeof(FBTExecuteMeleeAttackMemory);
}

void UBTTask_ExecuteMeleeAttack::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
    MyMemory->bIsExecutingAttack = false;
}

void UBTTask_ExecuteMeleeAttack::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
    FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
    MyMemory->bIsExecutingAttack = false;
}

EBTNodeResult::Type UBTTask_ExecuteMeleeAttack::
ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);
    MyMemory->bIsExecutingAttack = false;

    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;

    if (!AIController || !BlackboardComp || !ControlledPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteMeleeAttack: Missing AIController, Blackboard, or Pawn!"));
        return EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Log, TEXT("--- BTTask_ExecuteMeleeAttack: Task Started for %s ---"), *ControlledPawn->GetName());

    AActor* TargetActor = nullptr;
    if (!TargetActorKey.IsNone())
    {
        TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    }

    IMeleeAbilityExecutor* AbilityExecutor = Cast<IMeleeAbilityExecutor>(ControlledPawn);
    if (!AbilityExecutor)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteMeleeAttack: Controlled Pawn '%s' does not implement IMeleeAbilityExecutor interface!"), *ControlledPawn->GetName());
        return EBTNodeResult::Failed;
    }

    if (IMeleeAbilityExecutor::Execute_ExecuteMeleeAttack(ControlledPawn,CurrentAttackType, TargetActor))
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteMeleeAttack: Execute_ExecuteMeleeAttack returned true. Task InProgress for %s."), *ControlledPawn->GetName());
        MyMemory->bIsExecutingAttack = true;
        return EBTNodeResult::InProgress;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteMeleeAttack: Execute_ExecuteMeleeAttack returned false for %s."), *ControlledPawn->GetName());
        return EBTNodeResult::Failed;
    }
}

void UBTTask_ExecuteMeleeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);

    if (MyMemory->bIsExecutingAttack)
    {
        AAIController* AIController = OwnerComp.GetAIOwner();
        APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;

        if (!ControlledPawn)
        {
            UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteMeleeAttack::TickTask: Lost Controlled Pawn!"));
            MyMemory->bIsExecutingAttack = false;
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }

        ICombatStatusProvider* StatusProvider = Cast<ICombatStatusProvider>(ControlledPawn);
        if (!StatusProvider)
        {
            UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteMeleeAttack::TickTask: Controlled Pawn '%s' does not implement ICombatStatusProvider! Cannot check attack status."), *ControlledPawn->GetName());
            MyMemory->bIsExecutingAttack = false;
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }

        if (!ICombatStatusProvider::Execute_IsPerformingMeleeAttack(ControlledPawn))
        {
            UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteMeleeAttack::TickTask: IsPerformingMeleeAttack is false. Task Succeeded for %s."), *ControlledPawn->GetName());
            MyMemory->bIsExecutingAttack = false;
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteMeleeAttack::TickTask: Called while not in executing state?"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
}

EBTNodeResult::Type UBTTask_ExecuteMeleeAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FBTExecuteMeleeAttackMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteMeleeAttackMemory>(NodeMemory);

    if (MyMemory->bIsExecutingAttack)
    {
        MyMemory->bIsExecutingAttack = false;
    }

    UE_LOG(LogTemp, Log, TEXT("BTTask_ExecuteMeleeAttack: Task Aborted."));
    FinishLatentAbort(OwnerComp);
    return EBTNodeResult::Aborted;
}
