

#include "AI/BST/BTService_UpdateCombatStatus.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/HealthComponent.h"
#include "Engine/Engine.h" 
#include "Interfaces/AI/Status/CombatStatusProvider.h"


UBTService_UpdateCombatStatus::UBTService_UpdateCombatStatus()
{
    NodeName = "Update Combat Status";

  
}



void UBTService_UpdateCombatStatus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    AAIController* AIController = OwnerComp.GetAIOwner();

    if (!BlackboardComp || !AIController)
    {
        return;
    }

    
    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        
        if (!CanMeleeAttackKey.SelectedKeyName.IsNone()) BlackboardComp->SetValueAsBool(CanMeleeAttackKey.SelectedKeyName, false);
        if (!CanTeleportKey.SelectedKeyName.IsNone()) BlackboardComp->SetValueAsBool(CanTeleportKey.SelectedKeyName, false);
        if (!IsHealthLowKey.SelectedKeyName.IsNone()) BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, false);
        if (!DistanceToTargetKey.SelectedKeyName.IsNone()) BlackboardComp->ClearValue(DistanceToTargetKey.SelectedKeyName);
        return;
    }

    
    if (!TargetActorKey.SelectedKeyName.IsNone() && !DistanceToTargetKey.SelectedKeyName.IsNone())
    {
        AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
        if (TargetActor)
        {
            float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
            BlackboardComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);
        }
        else
        {
            BlackboardComp->ClearValue(DistanceToTargetKey.SelectedKeyName);
        }
    }
    


    
    bool bCanMelee = false;
    bool bCanTeleport = false;
    
    ICombatStatusProvider* StatusProvider = Cast<ICombatStatusProvider>(ControlledPawn);
    if (StatusProvider) 
    {
        
        bCanMelee = ICombatStatusProvider::Execute_CanPerformMeleeAttack(ControlledPawn); 
        bCanTeleport = ICombatStatusProvider::Execute_CanPerformTeleport(ControlledPawn); 
    }
    

    
    if (!CanMeleeAttackKey.SelectedKeyName.IsNone())
    {
        BlackboardComp->SetValueAsBool(CanMeleeAttackKey.SelectedKeyName, bCanMelee);
    }
    if (!CanTeleportKey.SelectedKeyName.IsNone())
    {
        BlackboardComp->SetValueAsBool(CanTeleportKey.SelectedKeyName, bCanTeleport);
    }
    


    
    bool bIsLow = false;
    
    UHealthComponent* HealthComp = ControlledPawn->FindComponentByClass<UHealthComponent>();
    if (HealthComp) 
    {
        float CurrentHealth = HealthComp->GetCurrentHealth();
        float MaxHealth = HealthComp->GetMaxHealth();
        
        bIsLow = (MaxHealth > 0 && (CurrentHealth / MaxHealth) <= HealthLowThreshold);
    }
    

    
    if (!IsHealthLowKey.SelectedKeyName.IsNone())
    {
        BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, bIsLow);
    }
    
}

