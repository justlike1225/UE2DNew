#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIControllerBase.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UBehaviorTree;

UCLASS()
class MY2DGAMEDESIGN_API AEnemyAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIControllerBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Team")
	FGenericTeamId TeamId = FGenericTeamId(1);

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	static const FName TargetActorKeyName;
	static const FName CanSeeTargetKeyName;
	static const FName SelfActorKeyName;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	UPROPERTY(Transient)
	TObjectPtr<UBehaviorTree> EnemyBehaviorTree;
	
	UFUNCTION()
	virtual void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	virtual void SetTargetActorOnBlackboard(AActor* TargetActor);
};
