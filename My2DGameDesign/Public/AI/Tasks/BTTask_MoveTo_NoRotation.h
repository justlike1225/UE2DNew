// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h" // 继承自 BlackboardBase 以方便访问黑板键
#include "BTTask_MoveTo_NoRotation.generated.h"

/**
 * 自定义行为树任务：移动到目标位置或 Actor，但不尝试旋转 Pawn。
 * 依赖 CharacterMovementComponent 的 bOrientRotationToMovement 和 bUseControllerDesiredRotation 被设置为 false。
 */
UCLASS()
class MY2DGAMEDESIGN_API UBTTask_MoveTo_NoRotation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	// 构造函数
	UBTTask_MoveTo_NoRotation();

protected:
	/** 定义移动任务完成时允许的距离误差范围 */
	UPROPERTY(EditAnywhere, Category = Node, meta = (ClampMin = "0.0"))
	float AcceptableRadius;

	/** (可选) 如果为 true，并且目标是 Actor，则在移动期间持续追踪 Actor 的位置 */
	UPROPERTY(EditAnywhere, Category = Node)
	bool bTrackMovingGoal;

	/** (可选) 如果为 true，即使目标点在导航网格之外，也会尝试移动到最近的可达点 */
	UPROPERTY(EditAnywhere, Category=Node)
	bool bAllowPartialPath;

	/** (可选) 如果为 true，停止移动时也会停止 AI 的焦点设置 (如果之前有设置的话) */
	UPROPERTY(EditAnywhere, Category=Node)
	bool bStopOnOverlap; // 这个名字可能不太直观，但与原MoveTo的StopOnOverlap对应，用于到达时停止

	// --- 核心函数覆盖 ---

	/** 任务执行入口 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 用于检查移动状态的 Tick 函数 */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 任务被中断时的处理 */
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	/** 辅助函数，用于请求移动 */
	EBTNodeResult::Type PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
};