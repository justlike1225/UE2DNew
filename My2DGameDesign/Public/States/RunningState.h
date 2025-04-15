// 文件: Public/States/RunningState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "RunningState.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API URunningState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;
	virtual void TickState_Implementation(float DeltaTime) override;

	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override;
	virtual void HandleJumpInputPressed_Implementation() override;
	virtual void HandleAttackInput_Implementation() override;
	virtual void HandleDashInput_Implementation() override;
	virtual void HandleRunInputReleased_Implementation() override; // 处理奔跑键松开
	virtual void HandleWalkingOffLedge_Implementation() override;
	virtual void OnExitState_Implementation() override;
};