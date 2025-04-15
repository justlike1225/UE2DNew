// 文件: Public/States/FallingState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "FallingState.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UFallingState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;
	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override; // Air control
	virtual void HandleAttackInput_Implementation() override;
	virtual void HandleDashInput_Implementation() override;
	virtual void HandleLanded_Implementation(const FHitResult& Hit) override; // Transition back to ground
	// Falling state usually doesn't handle Jump input or Tick transitions (waits for Landed)
};