// 文件: Private/States/IdleState.cpp
#include "States/IdleState.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "Components/HeroCombatComponent.h"
#include "Components/DashComponent.h"

// Include headers for states this state can transition TO
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
    Super::HandleMoveInput_Implementation(Value); // Call base implementation if needed
	const float MoveValue = Value.Get<float>();
	if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER && MovementComponent.IsValid() && HeroContext)
	{
		HeroContext->AddMovementInput(HeroContext->GetActorForwardVector(), MoveValue);
		TrySetState(UWalkingState::StaticClass());
	}
}

void UIdleState::HandleJumpInputPressed_Implementation()
{
    Super::HandleJumpInputPressed_Implementation(); // Call base implementation if needed
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

void UIdleState::HandleAttackInput_Implementation()
{
    Super::HandleAttackInput_Implementation(); // Call base implementation if needed
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

void UIdleState::HandleDashInput_Implementation()
{
    Super::HandleDashInput_Implementation(); // Call base implementation if needed
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

void UIdleState::HandleWalkingOffLedge_Implementation()
{
    Super::HandleWalkingOffLedge_Implementation(); // Call base implementation if needed
	TrySetState(UFallingState::StaticClass());
}

// OnExitState, TickState, and other handlers inherit default behavior from HeroStateBase