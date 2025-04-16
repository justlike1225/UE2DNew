
#include "States/JumpingState.h"
#include "Interfaces/Context/HeroStateContext.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h" 


#include "States/FallingState.h"
#include "States/IdleState.h"      
 

void UJumpingState::OnEnterState_Implementation()
{
    Super::OnEnterState_Implementation();
  
    
}

void UJumpingState::TickState_Implementation(float DeltaTime)
{
    Super::TickState_Implementation(DeltaTime);
    
    if (MovementComponent.IsValid() && MovementComponent->Velocity.Z <= 0.0f)
    {
        if(HeroContext) HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UFallingState::StaticClass());
    }
}

void UJumpingState::HandleMoveInput_Implementation(const FInputActionValue& Value)
{
    Super::HandleMoveInput_Implementation(Value);
    
    if (MovementComponent.IsValid() && HeroContext)
    {
        const float MoveValue = Value.Get<float>();
        if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER)
        {
          const  AActor* Owner = HeroContext->Execute_GetOwningActor(HeroContext.GetObject());
            if(Owner)
            {
                HeroContext->Execute_ApplyMovementInput(HeroContext.GetObject(), Owner->GetActorForwardVector(), MoveValue);
            }
        }
    }
}

void UJumpingState::HandleJumpInputReleased_Implementation()
{
    Super::HandleJumpInputReleased_Implementation();
    
    if(HeroContext)
    {
        HeroContext->Execute_PerformStopJumping(HeroContext.GetObject());
    }
}



void UJumpingState::HandleLanded_Implementation(const FHitResult& Hit)
{
    Super::HandleLanded_Implementation(Hit);
    
    if(HeroContext) HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UIdleState::StaticClass());
}


