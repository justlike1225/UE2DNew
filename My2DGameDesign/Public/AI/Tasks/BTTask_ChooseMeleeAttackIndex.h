// My2DGameDesign/Public/AI/Tasks/BTTask_ChooseMeleeAttackIndex.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h" // 继承 BlackboardBase 方便访问黑板
#include "BTTask_ChooseMeleeAttackIndex.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UBTTask_ChooseMeleeAttackIndex : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_ChooseMeleeAttackIndex();

protected:
	/** 任务执行函数 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 要写入的攻击索引的黑板键 */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector MeleeAttackIndexKey; // 确保这个 Key 在黑板中是 Int 类型

	/** 最小攻击索引 (通常是 1) */
	UPROPERTY(EditAnywhere, Category = Task)
	int32 MinAttackIndex = 1;

	/** 最大攻击索引 (对于攻击1和2，就是 2) */
	UPROPERTY(EditAnywhere, Category = Task)
	int32 MaxAttackIndex = 2;
};