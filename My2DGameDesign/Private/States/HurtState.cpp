// File: Private/States/HurtState.cpp
#include "States/HurtState.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/HealthComponent.h" // May need health component
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h" // May need anim listener

// Include headers for states this state can transition TO
#include "States/IdleState.h"
#include "States/FallingState.h"
#include "States/DeadState.h"


void UHurtState::OnEnterState_Implementation()
{
    Super::OnEnterState_Implementation();
    if (MovementComponent.IsValid())
    {
        MovementComponent->StopMovementImmediately();
        MovementComponent->Velocity = FVector::ZeroVector;
    }


    if (AnimListener)
    {
        // 重要：调用 AnimListener 的 OnTakeHit 接口函数
        // 这里的参数可能需要调整，取决于你的 AnimInstance 如何处理
        // 最关键的是 bInterruptsCurrentAction 设为 true，让 AnimInstance 跳转
        bool bInterruptsCurrentAction = true;
        // 伤害值和方向可能在这个状态不易获取，可以传递默认值
        float TempDamage = 1.0f; // 临时值，表示受到伤害
        FVector TempDirection = FVector::ZeroVector;
        AnimListener->Execute_OnTakeHit(AnimListener.GetObject(), TempDamage, TempDirection, bInterruptsCurrentAction);
    }
  
}

void UHurtState::OnExitState_Implementation()
{
    Super::OnExitState_Implementation();
  
}

// Hurt state ignores all player inputs
void UHurtState::HandleMoveInput_Implementation(const FInputActionValue& Value) { Super::HandleMoveInput_Implementation(Value); }
void UHurtState::HandleJumpInputPressed_Implementation() { Super::HandleJumpInputPressed_Implementation(); }
void UHurtState::HandleAttackInput_Implementation() { Super::HandleAttackInput_Implementation(); }
void UHurtState::HandleDashInput_Implementation() { Super::HandleDashInput_Implementation(); }
void UHurtState::HandleRunInputPressed_Implementation() { Super::HandleRunInputPressed_Implementation(); }
void UHurtState::HandleRunInputReleased_Implementation() { Super::HandleRunInputReleased_Implementation(); }

// Override TakeDamage to prevent re-entering Hurt state from Hurt state
void UHurtState::HandleTakeDamage_Implementation()
{
    Super::HandleTakeDamage_Implementation(); // Call base for potential death transition check? No, base transitions TO hurt.
    // Do nothing here, already in hurt state.
    // Still check for death though.
    if (HeroContext && HeroContext->GetHealthComponent() && HeroContext->GetHealthComponent()->IsDead())
	{
		HandleDeath_Implementation(); // Still allow dying while hurt
	}

}

// Called by HeroContext when AnimNotify signals recovery
void UHurtState::HandleHurtRecovery_Implementation()
{
    Super::HandleHurtRecovery_Implementation();
    TransitionToIdleOrFalling();
}

// Override Death handling just to be explicit (though base class does the same)
void UHurtState::HandleDeath_Implementation()
{
    Super::HandleDeath_Implementation(); // Base class transitions to DeadState
}

void UHurtState::HandleLanded_Implementation(const FHitResult& Hit)
{
    Super::HandleLanded_Implementation(Hit);
    // Optional: Landing might automatically trigger recovery in some games
    // HandleHurtRecovery();
    // Or just let the normal recovery logic handle it.
}

// Helper function
void UHurtState::TransitionToIdleOrFalling()
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