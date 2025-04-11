// My2DGameDesign/Public/AI/Tasks/BTTask_ExecuteMeleeAttack.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ExecuteMeleeAttack.generated.h"

// 前向声明
class UEnemyMeleeAttackComponent;

// 为这个任务定义节点内存结构，用于存储异步状态
struct FBTExecuteMeleeAttackMemory
{
	// 标记异步任务是否仍在执行中
	bool bIsExecutingAttack = false;
	// (可选) 可以存储对组件的引用或其他需要跨 Tick 保持的数据
	TWeakObjectPtr<UEnemyMeleeAttackComponent> MeleeCompPtr;
};


UCLASS()
class MY2DGAMEDESIGN_API UBTTask_ExecuteMeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ExecuteMeleeAttack();

protected:
	/** 任务开始执行 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 任务在 InProgress 状态时每帧调用 */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 当任务被中断时调用 */
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 返回此节点需要的内存大小 */
	virtual uint16 GetInstanceMemorySize() const override;

	/** 初始化节点内存 */
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;

	/** 清理节点内存 */
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;


	/** (可选) 黑板键选择器，用于获取目标 Actor */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector TargetActorKey;

private:
	/** 辅助函数，获取近战组件并进行检查 */
	UEnemyMeleeAttackComponent* GetMeleeComponent(UBehaviorTreeComponent& OwnerComp) const;
};