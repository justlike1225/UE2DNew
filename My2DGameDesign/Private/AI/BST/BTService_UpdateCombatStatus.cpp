#include "AI/BSt/BTService_UpdateCombatStatus.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Enemies/EvilCreature.h"
#include "Components/HealthComponent.h"
#include "Components/EnemyMeleeAttackComponent.h"
#include "Components/TeleportComponent.h"
#include "Kismet/KismetSystemLibrary.h"


UBTService_UpdateCombatStatus::UBTService_UpdateCombatStatus()
{
	NodeName = "Update Combat Status";


	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UBTService_UpdateCombatStatus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);


	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();


	if (!BlackboardComp)
	{
		return;
	}
	if (!AIController)
	{
		return;
	}

	AEvilCreature* ControlledPawn = Cast<AEvilCreature>(AIController->GetPawn());
	if (!ControlledPawn)
	{
		return;
	}


	UHealthComponent* HealthComp = ControlledPawn->GetHealthComponent();
	UEnemyMeleeAttackComponent* MeleeComp = ControlledPawn->GetMeleeAttackComponent();
	UTeleportComponent* TeleportComp = ControlledPawn->GetTeleportComponent();
	if (!HealthComp)
	{
	}
	if (!MeleeComp)
	{
	}
	if (!TeleportComp)
	{
	}


	if (!TargetActorKey.IsSet())
	{
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));


	if (TargetActor)
	{
		float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());


		if (!DistanceToTargetKey.IsSet())
		{
		}
		BlackboardComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);


		float ReadbackDistance = BlackboardComp->GetValueAsFloat(DistanceToTargetKey.SelectedKeyName);
	}
	else
	{
		BlackboardComp->ClearValue(DistanceToTargetKey.SelectedKeyName);
	}

	if (CanMeleeAttackKey.IsSet())
	{
		bool bCanMelee = MeleeComp ? MeleeComp->CanAttack() : false;
		BlackboardComp->SetValueAsBool(CanMeleeAttackKey.SelectedKeyName, bCanMelee);
	}
	

	if (CanTeleportKey.IsSet())
	{
		bool bCanTeleport = TeleportComp ? TeleportComp->CanTeleport() : false;
		BlackboardComp->SetValueAsBool(CanTeleportKey.SelectedKeyName, bCanTeleport);
	}
	

	if (IsHealthLowKey.IsSet())
	{
		if (HealthComp)
		{
			float CurrentHealth = HealthComp->GetCurrentHealth();
			float MaxHealth = HealthComp->GetMaxHealth();
			float Threshold = 0.3f;
			bool bIsLow = (MaxHealth > 0 && (CurrentHealth / MaxHealth) <= Threshold);
			BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, bIsLow);
		}
		else
		{
			BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, false);
		}
	}
	


}
