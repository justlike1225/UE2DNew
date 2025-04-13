// 文件路径: Public/AI/Tasks/BTTask_ApplyImpulse.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h" // 改为继承 BlackboardBase 以方便访问黑板
#include "BTTask_ApplyImpulse.generated.h"

/**
 * 行为树任务：给控制的 Pawn 施加一个冲量。
 * 可以配置为朝向黑板中的目标 Actor。
 */
UCLASS()
class MY2DGAMEDESIGN_API UBTTask_ApplyImpulse : public UBTTask_BlackboardBase // <--- 继承自 BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_ApplyImpulse();

	/**
	 * 要施加的冲量强度。
	 * 这个值需要在行为树编辑器中根据需要调整。
	 */
	UPROPERTY(EditAnywhere, Category = "Task Parameters", meta = (ClampMin = "0.0"))
	float ImpulseStrength = 300.0f;

	/**
	 * 是否将冲量视为速度变更 (Velocity Change)。
	 */
	UPROPERTY(EditAnywhere, Category = "Task Parameters")
	bool bVelocityChange = true;

	/**
	 * 指定黑板中存储目标 Actor 的键。冲量将朝向这个 Actor。
	 * !!! 必须在行为树编辑器中正确设置此项 !!!
	 */
	UPROPERTY(EditAnywhere, Category = Blackboard) // <--- Blackboard Category
	FBlackboardKeySelector TargetActorKey; // <--- 添加黑板键选择器

	// --- 核心执行函数 ---
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	virtual FString GetStaticDescription() const override;
};