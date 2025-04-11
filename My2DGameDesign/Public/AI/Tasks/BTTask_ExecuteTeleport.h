// My2DGameDesign/Public/AI/Tasks/BTTask_ExecuteTeleport.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h" // 可以继承它方便访问黑板
#include "BTTask_ExecuteTeleport.generated.h"

// 前向声明
class UTeleportComponent;
class UNavigationSystemV1; // 需要导航系统

// 节点内存
struct FBTExecuteTeleportMemory
{
    bool bIsExecutingTeleport = false;
    TWeakObjectPtr<UTeleportComponent> TeleportCompPtr;
};


UENUM(BlueprintType)
enum class ETeleportMode : uint8
{
	Offensive UMETA(DisplayName = "Offensive (Near Target)"), // 传送到目标附近
	Defensive UMETA(DisplayName = "Defensive (Away Target)"), // 传送到远离目标
    Random    UMETA(DisplayName = "Random (Within Range)")     // 随机传送
};

UCLASS()
class MY2DGAMEDESIGN_API UBTTask_ExecuteTeleport : public UBTTask_BlackboardBase // 继承 BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_ExecuteTeleport();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual uint16 GetInstanceMemorySize() const override;
    virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
    virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

	/** 定义传送模式 */
	UPROPERTY(EditAnywhere, Category = Task)
	ETeleportMode TeleportMode = ETeleportMode::Offensive;

	/** 黑板键：目标 Actor (进攻/防御模式需要) */
	UPROPERTY(EditAnywhere, Category = Blackboard, meta = (EditCondition = "TeleportMode != ETeleportMode::Random", EditConditionHides))
	FBlackboardKeySelector TargetActorKey;

    /** 黑板键：自身 Actor */
    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector SelfActorKey; // 确保这个键在 BB 中被正确设置

    /** 进攻模式下传送到目标身后/侧面的距离 */
    UPROPERTY(EditAnywhere, Category = Task, meta = (EditCondition = "TeleportMode == ETeleportMode::Offensive", EditConditionHides))
    float OffensiveOffsetDistance = 100.0f;

    /** 防御模式下传送到远离目标的距离 */
    UPROPERTY(EditAnywhere, Category = Task, meta = (EditCondition = "TeleportMode == ETeleportMode::Defensive", EditConditionHides))
    float DefensiveTeleportDistance = 500.0f;

    /** 随机传送模式下的最大半径 */
    UPROPERTY(EditAnywhere, Category = Task, meta = (EditCondition = "TeleportMode == ETeleportMode::Random", EditConditionHides))
    float RandomTeleportRadius = 600.0f;

    /** 传送位置查询的范围 (用于导航系统检查) */
    UPROPERTY(EditAnywhere, Category = Task)
    float LocationFindRadius = 150.0f;

private:
    /** 获取传送组件 */
    UTeleportComponent* GetTeleportComponent(UBehaviorTreeComponent& OwnerComp) const;
    /** 计算传送目标位置 */
    bool CalculateTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation) const;
};