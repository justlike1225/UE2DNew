// My2DGameDesign/Public/AI/Services/BTService_UpdateCombatStatus.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BlackboardComponent.h" // 需要 BlackboardComponent
#include "AIController.h"                     // 需要 AIController
#include "BTService_UpdateCombatStatus.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UBTService_UpdateCombatStatus : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateCombatStatus();

protected:
	/** 服务执行的主要函数 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/**
	 * 黑板键选择器：用于在行为树编辑器中指定要写入的目标 Actor 键。
	 * 标记为 EditAnywhere 允许在编辑器中为该服务的实例选择不同的黑板键。
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	/** 黑板键选择器：用于写入计算出的距离值 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DistanceToTargetKey;

	/** 黑板键选择器：用于写入近战攻击是否就绪 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CanMeleeAttackKey;

	/** 黑板键选择器：用于写入传送是否就绪 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CanTeleportKey;

	/** 黑板键选择器：用于写入是否处于低血量状态 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsHealthLowKey;

	/**
	 * (可选) 直接在服务中配置低血量阈值，或者也可以用黑板键读取
	 * UPROPERTY(EditAnywhere, Category = "AI | Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	 * float LowHealthThreshold = 0.3f;
	 */

};