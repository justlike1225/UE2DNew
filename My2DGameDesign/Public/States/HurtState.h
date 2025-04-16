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
	

	// Overrides event handling
	virtual void HandleTakeDamage_Implementation() override; // Ignore further damage state changes
	virtual void HandleHurtRecovery_Implementation() override; // Triggered externally to exit state
	

private:
	// Helper to determine next state after recovery
	void TransitionToIdleOrFalling();
};