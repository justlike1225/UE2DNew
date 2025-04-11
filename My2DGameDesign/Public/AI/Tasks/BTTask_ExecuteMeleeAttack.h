#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ExecuteMeleeAttack.generated.h"

class UEnemyMeleeAttackComponent;

struct FBTExecuteMeleeAttackMemory
{
	bool bIsExecutingAttack = false;
	TWeakObjectPtr<UEnemyMeleeAttackComponent> MeleeCompPtr;
};

UCLASS()
class MY2DGAMEDESIGN_API UBTTask_ExecuteMeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ExecuteMeleeAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	                              EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	                           EBTMemoryClear::Type CleanupType) const override;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector TargetActorKey;

private:
	UEnemyMeleeAttackComponent* GetMeleeComponent(UBehaviorTreeComponent& OwnerComp) const;
};
