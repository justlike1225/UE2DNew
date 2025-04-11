// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Tasks/BTTask_MoveTo_NoRotation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveTo_NoRotation::UBTTask_MoveTo_NoRotation()
{
	NodeName = "Move To No Rotation";

	// 正确添加黑板键过滤器（注意：这里使用 "BlackboardKey.SelectedKeyName" 会在编辑器运行时正确解析）
	BlackboardKey.AddVectorFilter(this, FName(TEXT("None")));
	BlackboardKey.AddObjectFilter(this, FName(TEXT("None")), AActor::StaticClass());

	// 设置默认参数
	AcceptableRadius = 5.0f;
	bTrackMovingGoal = true;
	bAllowPartialPath = true;
	bStopOnOverlap = true;

	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveTo_NoRotation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return PerformMoveTask(OwnerComp, NodeMemory);
}

EBTNodeResult::Type UBTTask_MoveTo_NoRotation::PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();

	if (!MyController || !MyBlackboard || !MyController->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	UPathFollowingComponent* PathFollowingComp = MyController->GetPathFollowingComponent();
	if (!PathFollowingComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTTask_MoveTo_NoRotation: Missing PathFollowingComponent on AIController %s!"), *GetNameSafe(MyController));
		return EBTNodeResult::Failed;
	}

	EPathFollowingRequestResult::Type RequestResult = EPathFollowingRequestResult::Failed;

	// 判断目标类型（Actor or Vector）
	UObject* KeyValue = MyBlackboard->GetValueAsObject(BlackboardKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(KeyValue);

	if (TargetActor)
	{
		if (bTrackMovingGoal)
		{
			RequestResult = MyController->MoveToActor(TargetActor, AcceptableRadius, bStopOnOverlap, true, bAllowPartialPath);
		}
		else
		{
			RequestResult = MyController->MoveToLocation(TargetActor->GetActorLocation(), AcceptableRadius, bStopOnOverlap, true, bAllowPartialPath);
		}
	}
	else
	{
		FVector TargetLocation = MyBlackboard->GetValueAsVector(BlackboardKey.SelectedKeyName);
		RequestResult = MyController->MoveToLocation(TargetLocation, AcceptableRadius, bStopOnOverlap, true, bAllowPartialPath);
	}

	// 处理移动结果
	switch (RequestResult)
	{
	case EPathFollowingRequestResult::RequestSuccessful:
		return EBTNodeResult::InProgress;

	case EPathFollowingRequestResult::AlreadyAtGoal:
		return EBTNodeResult::Succeeded;

	default:
		return EBTNodeResult::Failed;
	}
}

void UBTTask_MoveTo_NoRotation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (!MyController || !MyController->GetPathFollowingComponent())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	EPathFollowingStatus::Type Status = MyController->GetPathFollowingComponent()->GetStatus();

	if (Status == EPathFollowingStatus::Moving)
	{
		// 可选：在此处重新调用 PerformMoveTask() 以适应移动目标位置的变化
	}
	else if (Status == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

EBTNodeResult::Type UBTTask_MoveTo_NoRotation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	UPathFollowingComponent* PathFollowingComp = (MyController ? MyController->GetPathFollowingComponent() : nullptr);

	if (PathFollowingComp && PathFollowingComp->GetStatus() == EPathFollowingStatus::Moving)
	{
		PathFollowingComp->AbortMove(*this, FPathFollowingResultFlags::UserAbort | FPathFollowingResultFlags::NewRequest);
	}

	return EBTNodeResult::Aborted;
}
