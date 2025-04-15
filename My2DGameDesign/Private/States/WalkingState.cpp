// 文件: Private/States/WalkingState.cpp
#include "States/WalkingState.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "Components/HeroCombatComponent.h"
#include "Components/DashComponent.h"

// Include headers for states this state can transition TO
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
		MovementComponent->MaxWalkSpeed = HeroContext->GetCachedWalkSpeed();
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
        // Check if velocity magnitude is very small
        if (MovementComponent->Velocity.SizeSquared2D() < FMath::Square(KINDA_SMALL_NUMBER))
        {
		    TrySetState(UIdleState::StaticClass());
        }
    }
}

void UWalkingState::HandleMoveInput_Implementation(const FInputActionValue& Value)
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

void UWalkingState::HandleJumpInputPressed_Implementation()
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

void UWalkingState::HandleAttackInput_Implementation()
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

void UWalkingState::HandleDashInput_Implementation()
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

void UWalkingState::HandleRunInputPressed_Implementation()
{
    Super::HandleRunInputPressed_Implementation();
	TrySetState(URunningState::StaticClass());
}

void UWalkingState::HandleWalkingOffLedge_Implementation()
{
    Super::HandleWalkingOffLedge_Implementation();
	TrySetState(UFallingState::StaticClass());
}

