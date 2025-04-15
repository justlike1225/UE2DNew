// 文件: Private/States/JumpingState.cpp
#include "States/JumpingState.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "Components/HeroCombatComponent.h"
#include "Components/DashComponent.h"

// Include headers for states this state can transition TO
#include "States/FallingState.h"
#include "States/IdleState.h"
#include "States/WalkingState.h"
#include "States/AttackingState.h"
#include "States/DashingState.h"


void UJumpingState::OnEnterState_Implementation()
{
    Super::OnEnterState_Implementation();
    // Jump action was already called in the previous state
    // Animation was likely already requested in previous state
}

void UJumpingState::TickState_Implementation(float DeltaTime)
{
    Super::TickState_Implementation(DeltaTime);
    // If vertical velocity is no longer positive (or close to zero), transition to Falling
    if (MovementComponent.IsValid() && MovementComponent->Velocity.Z <= 0.0f) // Use <= 0 to catch peak/start of fall
    {
        TrySetState(UFallingState::StaticClass());
    }
}

void UJumpingState::HandleMoveInput_Implementation(const FInputActionValue& Value)
{
    Super::HandleMoveInput_Implementation(Value);
    // Apply air control
    if (MovementComponent.IsValid() && HeroContext)
    {
        const float MoveValue = Value.Get<float>();
        if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER)
        {
            HeroContext->AddMovementInput(HeroContext->GetActorForwardVector(), MoveValue);
        }
    }
}

void UJumpingState::HandleJumpInputReleased_Implementation()
{
    Super::HandleJumpInputReleased_Implementation();
    if(HeroContext)
    {
        HeroContext->StopJumping(); // Allow variable jump height
    }
}

void UJumpingState::HandleAttackInput_Implementation()
{
    Super::HandleAttackInput_Implementation();
    // Allow air attack
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

void UJumpingState::HandleDashInput_Implementation()
{
    Super::HandleDashInput_Implementation();
    // Allow air dash
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

void UJumpingState::HandleLanded_Implementation(const FHitResult& Hit)
{
    Super::HandleLanded_Implementation(Hit);
    // Should ideally not happen in Jumping state, but if it does, transition to ground
    TrySetState(UIdleState::StaticClass());
}

// Jumping state ignores Run input
// Inherits default TakeDamage and Death handling