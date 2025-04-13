#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Interfaces/AI/Abilities/MeleeAbilityExecutor.h"
#include "UObject/Interface.h"
#include "BTTask_ExecuteMeleeAttack.generated.h"

enum class  EEnemyMeleeAttackType : uint8;
// 前向声明接口
class UBehaviorTreeComponent;
class ICombatStatusProvider;
class IMeleeAbilityExecutor;

// 用于本任务的节点内存
USTRUCT()
struct FBTExecuteMeleeAttackMemory
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsExecutingAttack = false;
};

/**
 * 行为树任务：命令 AI 执行一次近战攻击。
 * 使用 IMeleeAbilityExecutor 接口触发攻击，
 * 使用 ICombatStatusProvider 接口判断攻击是否结束。
 */
UCLASS()
class MY2DGAMEDESIGN_API UBTTask_ExecuteMeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ExecuteMeleeAttack();

protected:
	UPROPERTY(EditAnywhere, Category = Task, meta = (DisplayName = "攻击类型")) // 让策划可以在 BT 编辑器里选
	EEnemyMeleeAttackType CurrentAttackType = EEnemyMeleeAttackType::Attack1; // 提供默认值

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

	/** 黑板键：目标 Actor */
	UPROPERTY(EditAnywhere, Category = Blackboard, meta = (DisplayName = "目标Actor"))
	FBlackboardKeySelector TargetActorKey;
};
