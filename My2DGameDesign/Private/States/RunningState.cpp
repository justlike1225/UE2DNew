
#include "States/RunningState.h"
#include "Interfaces/Context/HeroStateContext.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "GameFramework/Actor.h" 


#include "States/IdleState.h"
#include "States/WalkingState.h"
#include "States/JumpingState.h"
#include "States/FallingState.h"
   

void URunningState::OnEnterState_Implementation()
{
	Super::OnEnterState_Implementation();
	
	if (MovementComponent.IsValid() && HeroContext)
	{
		MovementComponent->MaxWalkSpeed = HeroContext->Execute_GetCachedRunSpeed(HeroContext.GetObject());
	}
    
    if(AnimListener)
    {
        AnimListener->Execute_OnIntentStateChanged(AnimListener.GetObject(), true, true); 
    }
}

void URunningState::TickState_Implementation(float DeltaTime)
{
	Super::TickState_Implementation(DeltaTime);
	
	if (MovementComponent.IsValid())
    {
        if (MovementComponent->Velocity.Size() < FMath::Square(KINDA_SMALL_NUMBER))
        {
            
            if(HeroContext) HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UIdleState::StaticClass());
        }
    }
}

void URunningState::HandleMoveInput_Implementation(const FInputActionValue& Value)
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
            
        const    AActor* Owner = HeroContext->Execute_GetOwningActor(HeroContext.GetObject());
            if(Owner)
            {
			    HeroContext->Execute_ApplyMovementInput(HeroContext.GetObject(), Owner->GetActorForwardVector(), MoveValue);
            }
		}
	}
}

void URunningState::HandleJumpInputPressed_Implementation()
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



void URunningState::HandleRunInputReleased_Implementation()
{
    Super::HandleRunInputReleased_Implementation();
    if (HeroContext)
    {
        
        if (MovementComponent.IsValid() && MovementComponent->Velocity.SizeSquared2D() > FMath::Square(KINDA_SMALL_NUMBER))
        {
            
            HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UWalkingState::StaticClass());
        }
        else
        {
            
            HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UIdleState::StaticClass());
        }
    }
}

void URunningState::HandleWalkingOffLedge_Implementation()
{
    Super::HandleWalkingOffLedge_Implementation();
    
	if(HeroContext) HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UFallingState::StaticClass());
}
