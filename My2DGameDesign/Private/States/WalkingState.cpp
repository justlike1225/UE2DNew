
#include "States/WalkingState.h"
#include "Interfaces/Context/HeroStateContext.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "GameFramework/Actor.h" 


#include "States/IdleState.h"
#include "States/RunningState.h"
#include "States/JumpingState.h"
#include "States/FallingState.h"
#include "States/AttackingState.h" 
#include "States/DashingState.h"   

void UWalkingState::OnEnterState_Implementation()
{
	Super::OnEnterState_Implementation();
	
	if (MovementComponent.IsValid() && HeroContext)
	{
		MovementComponent->MaxWalkSpeed = HeroContext->Execute_GetCachedWalkSpeed(HeroContext.GetObject());
	}
    
    if(AnimListener)
    {
        AnimListener->Execute_OnIntentStateChanged(AnimListener.GetObject(), true, false); 
    }
}

void UWalkingState::TickState_Implementation(float DeltaTime)
{
	Super::TickState_Implementation(DeltaTime);
	
	if (MovementComponent.IsValid())
    {
        if (MovementComponent->Velocity.SizeSquared2D() < FMath::Square(KINDA_SMALL_NUMBER))
        {
            
            if(HeroContext) HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UIdleState::StaticClass());
        }
    }
}

void UWalkingState::HandleMoveInput_Implementation(const FInputActionValue& Value)
{
    Super::HandleMoveInput_Implementation(Value);
	const float MoveValue = Value.Get<float>();

	if (HeroContext)
	{
		if (FMath::Abs(MoveValue) < KINDA_SMALL_NUMBER)
		{
            
            HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UIdleState::StaticClass());
		}
		else if (MovementComponent.IsValid()) 
		{
            
         const   AActor* Owner = HeroContext->Execute_GetOwningActor(HeroContext.GetObject());
            if(Owner)
            {
			    HeroContext->Execute_ApplyMovementInput(HeroContext.GetObject(), Owner->GetActorForwardVector(), MoveValue);
            }
		}
	}
}

void UWalkingState::HandleJumpInputPressed_Implementation()
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



void UWalkingState::HandleRunInputPressed_Implementation()
{
    Super::HandleRunInputPressed_Implementation();
    
	if(HeroContext) HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), URunningState::StaticClass());
}

void UWalkingState::HandleWalkingOffLedge_Implementation()
{
    Super::HandleWalkingOffLedge_Implementation();
    
	if(HeroContext) HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UFallingState::StaticClass());
}