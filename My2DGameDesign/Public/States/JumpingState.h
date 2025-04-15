// 文件: Public/States/JumpingState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "JumpingState.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UJumpingState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;
	virtual void TickState_Implementation(float DeltaTime) override; // Check for falling transition
	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override; // Air control
	virtual void HandleJumpInputReleased_Implementation() override; // StopJumping
	virtual void HandleAttackInput_Implementation() override;
	virtual void HandleDashInput_Implementation() override;
	virtual void HandleLanded_Implementation(const FHitResult& Hit) override; // Transition back to ground
};