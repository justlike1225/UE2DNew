#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h" // 只需要这个核心头文件
#include "BehaviorTree/BlackboardComponent.h" // BlackboardComponent 在 .cpp 中需要，但 .h 中可以只前向声明（如果需要）
#include "BTService_UpdateCombatStatus.generated.h"


UCLASS()
class MY2DGAMEDESIGN_API UBTService_UpdateCombatStatus : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateCombatStatus();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// --- 黑板键选择器属性 ---
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "目标 Actor")) // 添加中文显示名
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "到目标的距离"))
	FBlackboardKeySelector DistanceToTargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "能否近战攻击"))
	FBlackboardKeySelector CanMeleeAttackKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "能否传送"))
	FBlackboardKeySelector CanTeleportKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (DisplayName = "生命值是否过低"))
	FBlackboardKeySelector IsHealthLowKey;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Service Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "低生命阈值"))
	float HealthLowThreshold = 0.3f; 

    
};