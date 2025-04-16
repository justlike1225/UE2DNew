
#include "States/IdleState.h"
#include "Interfaces/Context/HeroStateContext.h"  
#include "GameFramework/CharacterMovementComponent.h" 
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h" 
#include "GameFramework/Actor.h" 


#include "States/WalkingState.h"
#include "States/JumpingState.h"
#include "States/AttackingState.h" 
#include "States/DashingState.h"   
#include "States/FallingState.h"

void UIdleState::OnEnterState_Implementation()
{
	Super::OnEnterState_Implementation(); 
	
	if (MovementComponent.IsValid())
	{
		MovementComponent->StopMovementImmediately();
        MovementComponent->Velocity = FVector::ZeroVector;
	}
	
	if (AnimListener)
	{
		AnimListener->Execute_OnIntentStateChanged(AnimListener.GetObject(), false, false);
	}
}

void UIdleState::HandleMoveInput_Implementation(const FInputActionValue& Value)
{
    Super::HandleMoveInput_Implementation(Value); 
	const float MoveValue = Value.Get<float>();

	
	if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER && HeroContext)
	{
		
		const AActor* Owner = HeroContext->Execute_GetOwningActor(HeroContext.GetObject());
		if(Owner && MovementComponent.IsValid()) 
        {
            
		    HeroContext->Execute_ApplyMovementInput(HeroContext.GetObject(), Owner->GetActorForwardVector(), MoveValue);
            
            HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UWalkingState::StaticClass());
        }
	}
}

void UIdleState::HandleJumpInputPressed_Implementation()
{
    Super::HandleJumpInputPressed_Implementation(); 
    
	if (HeroContext && MovementComponent.IsValid() && MovementComponent->IsMovingOnGround())
	{
        
		HeroContext->Execute_PerformJump(HeroContext.GetObject());
        
		HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UJumpingState::StaticClass());
        
		if (AnimListener)
		{
			AnimListener->Execute_OnJumpRequested(AnimListener.GetObject());
		}
	}
}



void UIdleState::HandleWalkingOffLedge_Implementation()
{
    Super::HandleWalkingOffLedge_Implementation(); 
    if (HeroContext)
    {
        
	    HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UFallingState::StaticClass());
    }
}