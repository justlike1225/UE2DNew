#include "AI/Tasks/BTTask_ExecuteTeleport.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "Interfaces/AI/Abilities/TeleportAbilityExecutor.h"
#include "Interfaces/AI/Status/CombatStatusProvider.h"

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

EBTNodeResult::Type UBTTask_ExecuteTeleport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	MyMemory->bIsExecutingTeleport = false;

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!ControlledPawn) return EBTNodeResult::Failed;

	ICombatStatusProvider* StatusProvider = Cast<ICombatStatusProvider>(ControlledPawn);
	if (StatusProvider && !ICombatStatusProvider::Execute_CanPerformTeleport(ControlledPawn))
	{
		return EBTNodeResult::Failed;
	}

	FVector TargetLocation;
	if (!CalculateTargetLocation(OwnerComp, TargetLocation))
	{
		return EBTNodeResult::Failed;
	}

	ITeleportAbilityExecutor* AbilityExecutor = Cast<ITeleportAbilityExecutor>(ControlledPawn);
	if (!AbilityExecutor)
	{
		return EBTNodeResult::Failed;
	}

	if (ITeleportAbilityExecutor::Execute_ExecuteTeleportToLocation(ControlledPawn, TargetLocation))
	{
		MyMemory->bIsExecutingTeleport = true;
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_ExecuteTeleport::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);

	if (MyMemory->bIsExecutingTeleport)
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;

		if (!ControlledPawn)
		{
			MyMemory->bIsExecutingTeleport = false;
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		ICombatStatusProvider* StatusProvider = Cast<ICombatStatusProvider>(ControlledPawn);
		if (!StatusProvider)
		{
			MyMemory->bIsExecutingTeleport = false;
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		if (!ICombatStatusProvider::Execute_IsPerformingTeleport(ControlledPawn))
		{
			MyMemory->bIsExecutingTeleport = false;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

EBTNodeResult::Type UBTTask_ExecuteTeleport::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	if (MyMemory->bIsExecutingTeleport)
	{
		MyMemory->bIsExecutingTeleport = false;
	}

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

	const float OriginalY = SelfPawn->GetActorLocation().Y;
	FNavLocation ResultLocation;
	bool bFoundLocation = false;
	FVector Origin = FVector::ZeroVector;

	AActor* TargetActor = nullptr;
	if (TeleportMode == ETeleportMode::Offensive || TeleportMode == ETeleportMode::Defensive)
	{
		TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
		if (!TargetActor) return false;
	}

	if (TeleportMode == ETeleportMode::Offensive)
	{
		Origin = TargetActor->GetActorLocation();
	}
	else if (TeleportMode == ETeleportMode::Defensive)
	{
		FVector SelfLocation = SelfPawn->GetActorLocation();
		FVector TargetLocation = TargetActor->GetActorLocation();
		FVector DirectionToTarget = (TargetLocation - SelfLocation).GetSafeNormal2D();
		Origin = SelfLocation - DirectionToTarget * DefensiveTeleportDistance;
	}
	else
	{
		Origin = SelfPawn->GetActorLocation();
	}

	float QueryRadius = (TeleportMode == ETeleportMode::Random) ? RandomTeleportRadius : LocationFindRadius;
	bFoundLocation = NavSys->GetRandomReachablePointInRadius(Origin, QueryRadius, ResultLocation);

	if (!bFoundLocation)
	{
		float FallbackRadius = RandomTeleportRadius;
		FVector FallbackOrigin = (TeleportMode == ETeleportMode::Offensive && TargetActor) ?
			TargetActor->GetActorLocation() : SelfPawn->GetActorLocation();

		bFoundLocation = NavSys->GetRandomReachablePointInRadius(FallbackOrigin, FallbackRadius, ResultLocation);
	}

	if (bFoundLocation)
	{
		ResultLocation.Location.Y = OriginalY;
		OutLocation = ResultLocation.Location;
		return true;
	}

	return false;
}
