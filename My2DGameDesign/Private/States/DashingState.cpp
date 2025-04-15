// File: Private/States/DashingState.cpp
#include "States/DashingState.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/DashComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"

// Include headers for states this state can transition TO
#include "States/IdleState.h"
#include "States/FallingState.h"


void UDashingState::OnEnterState_Implementation()
{
    Super::OnEnterState_Implementation();
    if (HeroContext)
    {
        DashComp = HeroContext->GetDashComponent();
        if (DashComp.IsValid())
        {
            // Bind to the Dash component's end event
            DashComp->OnDashEnded_Event.AddDynamic(this, &UDashingState::HandleDashEndInternal);
            // PerformDash was called by the previous state that transitioned here
        }
        else
        {
             // Dash component is missing, immediately exit?
             TransitionToIdleOrFalling();
        }
    }
    // Animation state was likely set by DashComponent->PerformDash()
}

void UDashingState::OnExitState_Implementation()
{
    Super::OnExitState_Implementation();
    // Unbind from the delegate to prevent issues if the state object is reused later
    if (DashComp.IsValid())
    {
        DashComp->OnDashEnded_Event.RemoveDynamic(this, &UDashingState::HandleDashEndInternal);
    }
    DashComp = nullptr; // Clear weak pointer cache
}

// Dashing state ignores most inputs
void UDashingState::HandleMoveInput_Implementation(const FInputActionValue& Value) { Super::HandleMoveInput_Implementation(Value); }
void UDashingState::HandleJumpInputPressed_Implementation() { Super::HandleJumpInputPressed_Implementation(); }
void UDashingState::HandleAttackInput_Implementation() { Super::HandleAttackInput_Implementation(); }
void UDashingState::HandleDashInput_Implementation() { Super::HandleDashInput_Implementation(); } // Already dashing
void UDashingState::HandleRunInputPressed_Implementation() { Super::HandleRunInputPressed_Implementation(); }
void UDashingState::HandleRunInputReleased_Implementation() { Super::HandleRunInputReleased_Implementation(); }


// This function is called by the DashComponent's delegate
void UDashingState::HandleDashEndInternal()
{
    TransitionToIdleOrFalling();
}


void UDashingState::HandleLanded_Implementation(const FHitResult& Hit)
{
    Super::HandleLanded_Implementation(Hit);
    // If landing happens while dashing, force the dash to end
    if (DashComp.IsValid())
    {
       
    }
    TransitionToIdleOrFalling(); // Transition immediately on landing
}

void UDashingState::HandleWalkingOffLedge_Implementation()
{
    Super::HandleWalkingOffLedge_Implementation();
    // If dashing off a ledge, force the dash to end?
    if (DashComp.IsValid())
    {
       // DashComp->EndDash(); // Optional: Force end dash logic
    }
    TransitionToIdleOrFalling(); // Will transition to Falling state
}

// Helper function
void UDashingState::TransitionToIdleOrFalling()
{
   if (MovementComponent.IsValid())
   {
        if (MovementComponent->IsFalling())
        {
            TrySetState(UFallingState::StaticClass());
        }
        else
        {
            TrySetState(UIdleState::StaticClass());
        }
   }
   else
   {
        TrySetState(UIdleState::StaticClass()); // Fallback
   }
}

// Inherits default TakeDamage and Death handling (will transition to Hurt/Dead)
// Note: Consider if Dash should grant invulnerability - if so, override HandleTakeDamage