// 文件: Public/States/DashingState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "DashingState.generated.h"

// 前向声明
class UDashComponent;

UCLASS()
class MY2DGAMEDESIGN_API UDashingState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;
	virtual void OnExitState_Implementation() override;

	
	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override;
	virtual void HandleJumpInputPressed_Implementation() override;
	virtual void HandleAttackInput_Implementation() override;
	virtual void HandleDashInput_Implementation() override; 
	virtual void HandleRunInputPressed_Implementation() override;
	virtual void HandleRunInputReleased_Implementation() override;
	
	UFUNCTION()
	void HandleDashEndInternal();


	virtual void HandleLanded_Implementation(const FHitResult& Hit) override;
	virtual void HandleWalkingOffLedge_Implementation() override;

protected:
	UPROPERTY()
	TWeakObjectPtr<UDashComponent> DashComp; 

private:
	
	void TransitionToIdleOrFalling();
};