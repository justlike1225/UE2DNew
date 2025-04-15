// File: Private/States/DeadState.cpp
#include "States/DeadState.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "GameFramework/CharacterMovementComponent.h" // Included for completeness, though movement is stopped

void UDeadState::OnEnterState_Implementation()
{
	Super::OnEnterState_Implementation();
	// HeroContext likely already disabled input, collision, movement in HandleDeath

	// Trigger Death animation via AnimListener
	if (AnimListener)
	{
        // Killer info might need to be passed into the state or stored in HeroContext?
        // For now, assume AnimInstance handles the specific death anim based on internal state
		AnimListener->Execute_OnDeathState(AnimListener.GetObject(), nullptr); // Pass nullptr for Killer for now
	}
   
}


void UDeadState::HandleMoveInput_Implementation(const FInputActionValue& Value) { Super::HandleMoveInput_Implementation(Value); }
void UDeadState::HandleJumpInputPressed_Implementation() { Super::HandleJumpInputPressed_Implementation(); }
void UDeadState::HandleJumpInputReleased_Implementation() { Super::HandleJumpInputReleased_Implementation(); }
void UDeadState::HandleAttackInput_Implementation() { Super::HandleAttackInput_Implementation(); }
void UDeadState::HandleDashInput_Implementation() { Super::HandleDashInput_Implementation(); }
void UDeadState::HandleRunInputPressed_Implementation() { Super::HandleRunInputPressed_Implementation(); }
void UDeadState::HandleRunInputReleased_Implementation() { Super::HandleRunInputReleased_Implementation(); }
void UDeadState::HandleLanded_Implementation(const FHitResult& Hit) { Super::HandleLanded_Implementation(Hit); }
void UDeadState::HandleWalkingOffLedge_Implementation() { Super::HandleWalkingOffLedge_Implementation(); }
void UDeadState::HandleTakeDamage_Implementation() { Super::HandleTakeDamage_Implementation(); } 
void UDeadState::HandleHurtRecovery_Implementation() { Super::HandleHurtRecovery_Implementation(); } 
void UDeadState::HandleDeath_Implementation() { Super::HandleDeath_Implementation(); } 