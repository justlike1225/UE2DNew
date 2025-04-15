// 文件: Public/States/HurtState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "HurtState.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UHurtState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;
	virtual void OnExitState_Implementation() override;

	// Hurt state ignores all inputs
	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override;
	virtual void HandleJumpInputPressed_Implementation() override;
	virtual void HandleAttackInput_Implementation() override;
	virtual void HandleDashInput_Implementation() override;
	virtual void HandleRunInputPressed_Implementation() override;
	virtual void HandleRunInputReleased_Implementation() override;

	// Overrides event handling
	virtual void HandleTakeDamage_Implementation() override; // Ignore further damage state changes
	virtual void HandleHurtRecovery_Implementation() override; // Triggered externally to exit state
	virtual void HandleDeath_Implementation() override; // Transition to DeadState
	virtual void HandleLanded_Implementation(const FHitResult& Hit) override; // Might happen if knocked into air

private:
	// Helper to determine next state after recovery
	void TransitionToIdleOrFalling();
};