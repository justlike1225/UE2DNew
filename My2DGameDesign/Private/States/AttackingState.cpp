

// File: Private/States/AttackingState.cpp
#include "States/AttackingState.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/HeroCombatComponent.h"
#include "Components/DashComponent.h" // Needed for dash input handling
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h" // Needed for animation calls

// Include headers for states this state can transition TO
#include "States/IdleState.h"
#include "States/FallingState.h"
#include "States/JumpingState.h"
#include "States/DashingState.h" // Can be interrupted by dash?


void UAttackingState::OnEnterState_Implementation()
{
    Super::OnEnterState_Implementation();
    bIsAirAttack = false; // Default to ground

    if (HeroContext)
    {
        CombatComp = HeroContext->GetHeroCombatComponent(); // Cache component
        if (MovementComponent.IsValid())
        {
            bIsAirAttack = MovementComponent->IsFalling();
            // Stop horizontal movement during ground attacks
            if (!bIsAirAttack)
            {
                 MovementComponent->Velocity.X = 0.0f;
                 MovementComponent->Velocity.Y = 0.0f; // Ensure 2D stop
                 // HeroCombatComponent->PerformGroundCombo() was called by previous state
            }
            else
            {
                 // HeroCombatComponent->PerformAirAttack() was called by previous state
            }
        }
        // Note: CombatComponent should handle informing the AnimListener about combo count / air attack start
    }
}

void UAttackingState::OnExitState_Implementation()
{
    Super::OnExitState_Implementation();
    // Clean up or reset combat state if needed, though component/notifies usually handle this.
     CombatComp = nullptr; // Clear weak pointer cache
}

void UAttackingState::HandleAttackInput_Implementation()
{
    Super::HandleAttackInput_Implementation();
    // Handle combo input during ground attacks
    if (!bIsAirAttack && CombatComp.IsValid() && CombatComp->CanCombo())
    {
        CombatComp->PerformGroundCombo(); // Request next attack in combo sequence
        // Stay in AttackingState
    }
    // Air attacks typically don't combo this way
}

// Attacking state usually ignores other inputs or handles them differently
void UAttackingState::HandleJumpInputPressed_Implementation()
{
    Super::HandleJumpInputPressed_Implementation();
  
}

void UAttackingState::HandleDashInput_Implementation()
{
    Super::HandleDashInput_Implementation();
    // Option 1: Ignore Dash
    // Option 2: Allow Dash to interrupt attack
    if (HeroContext)
    {
        UDashComponent* DashComp = HeroContext->GetDashComponent();
	    if (DashComp && DashComp->CanDash())
	    {
            // Interrupt current action if needed (CombatComponent listens for this)
            HeroContext->BroadcastActionInterrupt_Implementation();
            // Perform Dash
		    DashComp->PerformDash();
            TrySetState(UDashingState::StaticClass());
	    }
    }
}

// Ignore Move input during attack (except maybe air control during air attack?)
void UAttackingState::HandleMoveInput_Implementation(const FInputActionValue& Value)
{
    Super::HandleMoveInput_Implementation(Value);
    if (bIsAirAttack && MovementComponent.IsValid() && HeroContext)
    {
        // Apply air control if desired during air attack
        const float MoveValue = Value.Get<float>();
        if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER)
        {
            HeroContext->AddMovementInput(HeroContext->GetActorForwardVector(), MoveValue);
        }
    }
}


void UAttackingState::HandleAttackEnd_Implementation()
{
    Super::HandleAttackEnd_Implementation();
    TransitionToIdleOrFalling();
}

void UAttackingState::HandleLanded_Implementation(const FHitResult& Hit)
{
    Super::HandleLanded_Implementation(Hit);
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Landed in AttackingState"));
    if (bIsAirAttack)
    {
        // Option 1: Finish attack immediately
         TransitionToIdleOrFalling();
       
    }
   
}

void UAttackingState::HandleWalkingOffLedge_Implementation()
{
    Super::HandleWalkingOffLedge_Implementation();
    // If a ground attack walks off a ledge
    if (!bIsAirAttack)
    {
        bIsAirAttack = true; // Now it's an air attack (sort of)
        // Option 1: Interrupt attack
         //HeroContext->BroadcastActionInterrupt_Implementation();
         //TrySetState(UFallingState::StaticClass());
        // Option 2: Allow animation to finish, then fall
        // Let HandleAttackEnd handle the transition to FallingState after anim finishes
    }
    // If already air attacking, do nothing extra, will transition to Falling on HandleAttackEnd
}

// Helper function to transition to Idle or Falling state
void UAttackingState::TransitionToIdleOrFalling()
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

