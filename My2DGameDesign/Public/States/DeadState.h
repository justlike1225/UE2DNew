// 文件: Public/States/DeadState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "DeadState.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UDeadState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;

	// Dead state ignores all inputs and events
	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override;
	virtual void HandleJumpInputPressed_Implementation() override;
	virtual void HandleJumpInputReleased_Implementation() override;
	virtual void HandleAttackInput_Implementation() override;
	virtual void HandleDashInput_Implementation() override;
	virtual void HandleRunInputPressed_Implementation() override;
	virtual void HandleRunInputReleased_Implementation() override;
	virtual void HandleLanded_Implementation(const FHitResult& Hit) override;
	virtual void HandleWalkingOffLedge_Implementation() override;
	virtual void HandleTakeDamage_Implementation() override;
	virtual void HandleHurtRecovery_Implementation() override;
	virtual void HandleDeath_Implementation() override; // Already dead
};