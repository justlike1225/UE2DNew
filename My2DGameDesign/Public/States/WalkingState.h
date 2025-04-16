// 文件: Public/States/WalkingState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "WalkingState.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UWalkingState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;
	virtual void TickState_Implementation(float DeltaTime) override; // 需要Tick来检查是否停止
	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override;
	virtual void HandleJumpInputPressed_Implementation() override;
	virtual void HandleRunInputPressed_Implementation() override;
	virtual void HandleWalkingOffLedge_Implementation() override;
	
};