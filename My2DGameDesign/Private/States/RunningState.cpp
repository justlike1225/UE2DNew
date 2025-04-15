// 文件: Private/States/RunningState.cpp
#include "States/RunningState.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "Components/HeroCombatComponent.h"
#include "Components/DashComponent.h"

// Include headers for states this state can transition TO
#include "States/IdleState.h"
#include "States/WalkingState.h"
#include "States/JumpingState.h"
#include "States/FallingState.h"
#include "States/AttackingState.h"
#include "States/DashingState.h"

void URunningState::OnEnterState_Implementation()
{
	Super::OnEnterState_Implementation();
	if (MovementComponent.IsValid() && HeroContext)
	{
		MovementComponent->MaxWalkSpeed = HeroContext->GetCachedRunSpeed();
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
        // Check if velocity magnitude is very small
        if (MovementComponent->Velocity.SizeSquared2D() < FMath::Square(KINDA_SMALL_NUMBER))
        {
		    TrySetState(UIdleState::StaticClass());
        }
    }
}

void URunningState::HandleMoveInput_Implementation(const FInputActionValue& Value)
{
    Super::HandleMoveInput_Implementation(Value);
	const float MoveValue = Value.Get<float>();
	if (FMath::Abs(MoveValue) < KINDA_SMALL_NUMBER)
	{
        TrySetState(UIdleState::StaticClass());
	}
	else if (MovementComponent.IsValid() && HeroContext)
	{
		HeroContext->AddMovementInput(HeroContext->GetActorForwardVector(), MoveValue);
	}
}

void URunningState::HandleJumpInputPressed_Implementation()
{
    Super::HandleJumpInputPressed_Implementation();
	if (MovementComponent.IsValid() && MovementComponent->IsMovingOnGround() && HeroContext)
	{
		HeroContext->Jump();
		TrySetState(UJumpingState::StaticClass());
		if (AnimListener)
		{
			AnimListener->Execute_OnJumpRequested(AnimListener.GetObject());
		}
	}
}

void URunningState::HandleAttackInput_Implementation()
{
    Super::HandleAttackInput_Implementation();
	if (HeroContext)
    {
        UHeroCombatComponent* CombatComp = HeroContext->GetHeroCombatComponent();
	    if (CombatComp && CombatComp->CanCombo())
	    {
		    CombatComp->PerformGroundCombo();
		    TrySetState(UAttackingState::StaticClass());
	    }
    }
}

void URunningState::HandleDashInput_Implementation()
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

void URunningState::HandleRunInputReleased_Implementation()
{
    Super::HandleRunInputReleased_Implementation();
    // If still moving, transition to Walking, otherwise Idle handled by Tick/MoveInput
    if (MovementComponent.IsValid() && MovementComponent->Velocity.SizeSquared2D() > FMath::Square(KINDA_SMALL_NUMBER))
    {
	    TrySetState(UWalkingState::StaticClass());
    }
    else
    {
        TrySetState(UIdleState::StaticClass());
    }
}

void URunningState::HandleWalkingOffLedge_Implementation()
{
    Super::HandleWalkingOffLedge_Implementation();
	TrySetState(UFallingState::StaticClass());
}

void URunningState::OnExitState_Implementation()
{
    Super::OnExitState_Implementation();
    // Reset speed to walk speed when exiting run state, unless going to Dash/Attack maybe?
    // Let the entering state handle setting the correct speed.
     if(AnimListener)
     {
        // Reset intent, EnterState of next state will set correct one
        // AnimListener->Execute_OnIntentStateChanged(AnimListener.GetObject(), false, false);
     }
}