#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ExecuteTeleport.generated.h"

class UTeleportComponent;
class UNavigationSystemV1;

struct FBTExecuteTeleportMemory
{
	bool bIsExecutingTeleport = false;
	TWeakObjectPtr<UTeleportComponent> TeleportCompPtr;
};

UENUM(BlueprintType)
enum class ETeleportMode : uint8
{
	Offensive UMETA(DisplayName = "Offensive (Near Target)"),
	Defensive UMETA(DisplayName = "Defensive (Away Target)"),
	Random UMETA(DisplayName = "Random (Within Range)")
};

UCLASS()
class MY2DGAMEDESIGN_API UBTTask_ExecuteTeleport : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_ExecuteTeleport();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	                              EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	                           EBTMemoryClear::Type CleanupType) const override;

	UPROPERTY(EditAnywhere, Category = Task)
	ETeleportMode TeleportMode = ETeleportMode::Offensive;

	UPROPERTY(EditAnywhere, Category = Blackboard,
		meta = (EditCondition = "TeleportMode != ETeleportMode::Random", EditConditionHides))
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector SelfActorKey;

	UPROPERTY(EditAnywhere, Category = Task,
		meta = (EditCondition = "TeleportMode == ETeleportMode::Offensive", EditConditionHides))
	float OffensiveOffsetDistance = 100.0f;

	UPROPERTY(EditAnywhere, Category = Task,
		meta = (EditCondition = "TeleportMode == ETeleportMode::Defensive", EditConditionHides))
	float DefensiveTeleportDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = Task,
		meta = (EditCondition = "TeleportMode == ETeleportMode::Random", EditConditionHides))
	float RandomTeleportRadius = 600.0f;

	UPROPERTY(EditAnywhere, Category = Task)
	float LocationFindRadius = 150.0f;

private:
	
	bool CalculateTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation) const;
};
