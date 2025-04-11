#pragma once // 防止头文件被重复包含

#include "CoreMinimal.h" // UE4/5 核心功能
#include "BehaviorTree/BTService.h" // 继承自 BTService
#include "BehaviorTree/BehaviorTreeComponent.h" // 需要 UBehaviorTreeComponent
#include "BTService_UpdateFacingDirection.generated.h" // UE 生成的代码

/**
 * 行为树服务 (Service)，用于在AI移动时周期性地更新其视觉朝向。
 * 它会根据黑板中的目标Actor位置，调用拥有者Pawn的SetFacingDirection函数。
 * 需要将此服务添加到行为树中包含原生 "Move To" 任务的 Sequence 或 Selector 节点上。
 */
UCLASS()
class MY2DGAMEDESIGN_API UBTService_UpdateFacingDirection : public UBTService
{
	GENERATED_BODY() // UE 必需的宏

public:
	// 构造函数
	UBTService_UpdateFacingDirection();

protected:
	/**
	 * 黑板键选择器 (Blackboard Key Selector)
	 * 用于在行为树编辑器中指定哪个黑板键存储着目标 Actor (通常是玩家)。
	 * 必须在编辑器中正确设置此项！
	 */
	UPROPERTY(EditAnywhere, Category = Blackboard) // 允许在编辑器中设置
	FBlackboardKeySelector TargetActorKey;

	/**
	 * Service 的核心逻辑函数，会根据 Interval 设置周期性调用。
	 * @param OwnerComp 行为树组件
	 * @param NodeMemory 该节点的内存空间
	 * @param DeltaSeconds 自上次调用以来的时间差
	 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};