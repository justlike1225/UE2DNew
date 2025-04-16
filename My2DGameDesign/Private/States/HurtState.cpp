
#include "States/HurtState.h"
#include "Interfaces/Context/HeroStateContext.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/HealthComponent.h" 
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h" 


#include "States/IdleState.h"
#include "States/FallingState.h"
#include "States/DeadState.h"


void UHurtState::OnEnterState_Implementation()
{
    Super::OnEnterState_Implementation();
    
    if (MovementComponent.IsValid())
    {
        MovementComponent->StopMovementImmediately();
        MovementComponent->Velocity = FVector::ZeroVector;
    }

    
    if (AnimListener)
    {
        bool bInterruptsCurrentAction = true; 
        float TempDamage = 1.0f;           
        FVector TempDirection = FVector::ZeroVector; 
        AnimListener->Execute_OnTakeHit(AnimListener.GetObject(), TempDamage, TempDirection, bInterruptsCurrentAction);
    }
}







void UHurtState::HandleTakeDamage_Implementation()
{
    
    if (HealthComponent.IsValid() && HealthComponent->IsDead())
	{
        Super::HandleDeath_Implementation(); 
	}
    
}


void UHurtState::HandleHurtRecovery_Implementation()
{
    Super::HandleHurtRecovery_Implementation();
    
    TransitionToIdleOrFalling();
}





void UHurtState::TransitionToIdleOrFalling()
{
   if (HeroContext && MovementComponent.IsValid())
   {
        if (MovementComponent->IsFalling())
        {
            HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UFallingState::StaticClass());
        }
        else
        {
            HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UIdleState::StaticClass());
        }
   }
   else if(HeroContext) 
   {
        HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UIdleState::StaticClass());
   }
}