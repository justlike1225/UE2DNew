// 文件: Private/States/FallingState.cpp
#include "States/FallingState.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/HeroCombatComponent.h"
#include "Components/DashComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"

// Include headers for states this state can transition TO
#include "States/IdleState.h"
#include "States/WalkingState.h"
#include "States/AttackingState.h"
#include "States/DashingState.h"


void UFallingState::OnEnterState_Implementation()
{
    Super::OnEnterState_Implementation();
    // Notify animation instance about falling state if necessary
    // Often, the animation blueprint handles this based on IsFalling()
     if(AnimListener)
     {
     	// You might not need specific Enter/Exit calls for Falling
         // if the AnimBP checks MovementComponent->IsFalling()
         // AnimListener->Execute_OnFallingStateChanged(AnimListener.GetObject(), true);
     }
}

void UFallingState::HandleMoveInput_Implementation(const FInputActionValue& Value)
{
    Super::HandleMoveInput_Implementation(Value);
    // Apply air control if movement component and context are valid
    if (MovementComponent.IsValid() && HeroContext)
    {
        const float MoveValue = Value.Get<float>();
        if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER)
        {
            HeroContext->AddMovementInput(HeroContext->GetActorForwardVector(), MoveValue);
        }
    }
}

void UFallingState::HandleAttackInput_Implementation()
{
    Super::HandleAttackInput_Implementation();
    if (HeroContext)
    {
        UHeroCombatComponent* CombatComp = HeroContext->GetHeroCombatComponent();
        if (CombatComp && CombatComp->CanAirAttack())
        {
            CombatComp->PerformAirAttack();
            TrySetState(UAttackingState::StaticClass());
        }
    }
}

void UFallingState::HandleDashInput_Implementation()
{
    Super::HandleDashInput_Implementation();
	if (HeroContext)
    {
        UDashComponent* DashComp = HeroContext->GetDashComponent();
	    if (DashComp && DashComp->CanDash())
	    {
		    DashComp->PerformDash();
            TrySetState(UDashingState::StaticClass());
	    }
    }
}

void UFallingState::HandleLanded_Implementation(const FHitResult& Hit)
{
    Super::HandleLanded_Implementation(Hit);
    if (HeroContext)
    {
       
        TrySetState(UIdleState::StaticClass());

    }
    else
    {
        TrySetState(UIdleState::StaticClass()); // Fallback to Idle
    }
}

