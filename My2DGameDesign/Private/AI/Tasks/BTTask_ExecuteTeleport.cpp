#include "AI/Tasks/BTTask_ExecuteTeleport.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Enemies/EvilCreature.h"
#include "Components/TeleportComponent.h"
#include "NavigationSystem.h"

UBTTask_ExecuteTeleport::UBTTask_ExecuteTeleport()
{
	NodeName = "Execute Teleport";
	bNotifyTick = true;
	bNotifyTaskFinished = true;


	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ExecuteTeleport, SelfActorKey),
	                              AActor::StaticClass());
}

uint16 UBTTask_ExecuteTeleport::GetInstanceMemorySize() const
{
	return sizeof(FBTExecuteTeleportMemory);
}

void UBTTask_ExecuteTeleport::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                               EBTMemoryInit::Type InitType) const
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	MyMemory->bIsExecutingTeleport = false;
	MyMemory->TeleportCompPtr = nullptr;
}

void UBTTask_ExecuteTeleport::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                            EBTMemoryClear::Type CleanupType) const
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	MyMemory->bIsExecutingTeleport = false;
	MyMemory->TeleportCompPtr = nullptr;
}


EBTNodeResult::Type UBTTask_ExecuteTeleport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);
	MyMemory->bIsExecutingTeleport = false;

	UTeleportComponent* TeleportComp = GetTeleportComponent(OwnerComp);
	if (!TeleportComp || !TeleportComp->CanTeleport())
	{
		return EBTNodeResult::Failed;
	}

	FVector TargetLocation;
	if (!CalculateTargetLocation(OwnerComp, TargetLocation))
	{
		return EBTNodeResult::Failed;
	}


	if (TeleportComp->ExecuteTeleport(TargetLocation))
	{
		MyMemory->bIsExecutingTeleport = true;
		MyMemory->TeleportCompPtr = TeleportComp;
		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Failed;
}

void UBTTask_ExecuteTeleport::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTExecuteTeleportMemory* MyMemory = CastInstanceNodeMemory<FBTExecuteTeleportMemory>(NodeMemory);

	if (MyMemory->bIsExecutingTeleport)
	{
		UTeleportComponent* TeleportComp = MyMemory->TeleportCompPtr.Get();

		if (TeleportComp && !TeleportComp->IsTeleporting())
		{
			MyMemory->bIsExecutingTeleport = false;
			MyMemory->TeleportCompPtr = nullptr;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else if (!TeleportComp)
		{
			MyMemory->bIsExecutingTeleport = false;
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
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
		UTeleportComponent* TeleportComp = MyMemory->TeleportCompPtr.Get();


		MyMemory->bIsExecutingTeleport = false;
		MyMemory->TeleportCompPtr = nullptr;
	}
	FinishLatentAbort(OwnerComp);
	return EBTNodeResult::Aborted;
}


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


bool UBTTask_ExecuteTeleport::CalculateTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation) const
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* SelfPawn = AIController ? AIController->GetPawn() : nullptr;


	if (!BlackboardComp || !SelfPawn)
	{
		return false;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return false;
	}


	const float OriginalY = SelfPawn->GetActorLocation().Y - 0.01f;

	FNavLocation ResultLocation;
	bool bFoundLocation = false;
	FVector Origin;


	if (TeleportMode == ETeleportMode::Offensive || TeleportMode == ETeleportMode::Defensive)
	{
		AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
		if (!TargetActor)
		{
			return false;
		}

		FVector SelfLocation = SelfPawn->GetActorLocation();
		FVector TargetLocation = TargetActor->GetActorLocation();
		FVector DirectionToTarget = (TargetLocation - SelfLocation).GetSafeNormal2D();

		if (TeleportMode == ETeleportMode::Offensive)
		{
			FVector TargetBackDirection = TargetActor->GetActorForwardVector().GetSafeNormal2D() * -1.0f;
			Origin = TargetLocation + TargetBackDirection * OffensiveOffsetDistance;
		}
		else
		{
			Origin = SelfLocation + (DirectionToTarget * -1.0f) * DefensiveTeleportDistance;
		}
	}
	else
	{
		FVector SelfLocation = SelfPawn->GetActorLocation();
		Origin = SelfLocation;
	}


	bFoundLocation = NavSys->GetRandomReachablePointInRadius(Origin, LocationFindRadius, ResultLocation);


	if (!bFoundLocation)
	{
		if (TeleportMode == ETeleportMode::Offensive)
		{
			AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
			if (TargetActor)
			{
				bFoundLocation = NavSys->GetRandomReachablePointInRadius(
					TargetActor->GetActorLocation(), RandomTeleportRadius, ResultLocation);
			}
		}
		else
		{
			bFoundLocation = NavSys->GetRandomReachablePointInRadius(SelfPawn->GetActorLocation(), RandomTeleportRadius,
			                                                         ResultLocation);
		}
	}


	if (bFoundLocation)
	{
		ResultLocation.Location.Y = OriginalY;

		OutLocation = ResultLocation.Location;
		return true;
	}

	return false;
}
