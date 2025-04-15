// 文件: Public/States/IdleState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "IdleState.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UIdleState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;
	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override;
	virtual void HandleJumpInputPressed_Implementation() override;
	virtual void HandleAttackInput_Implementation() override;
	virtual void HandleDashInput_Implementation() override;
	virtual void HandleWalkingOffLedge_Implementation() override;
	
};