
#include "States/FallingState.h"
#include "Interfaces/Context/HeroStateContext.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h" 
#include "GameFramework/Actor.h" 


#include "States/IdleState.h"      
#include "States/AttackingState.h" 
#include "States/DashingState.h"   


void UFallingState::HandleMoveInput_Implementation(const FInputActionValue& Value)
{
    Super::HandleMoveInput_Implementation(Value);
    
    if (MovementComponent.IsValid() && HeroContext)
    {
        const float MoveValue = Value.Get<float>();
        if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER)
        {
         const   AActor* Owner = HeroContext->Execute_GetOwningActor(HeroContext.GetObject());
            if(Owner)
            {
                HeroContext->Execute_ApplyMovementInput(HeroContext.GetObject(), Owner->GetActorForwardVector(), MoveValue);
            }
        }
    }
}


void UFallingState::HandleLanded_Implementation(const FHitResult& Hit)
{
    Super::HandleLanded_Implementation(Hit);
    if (HeroContext)
    {
        
        HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), UIdleState::StaticClass());
        
        
        
    }
}


