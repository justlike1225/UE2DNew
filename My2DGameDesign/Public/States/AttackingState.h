// 文件: Public/States/AttackingState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "AttackingState.generated.h"

// 前向声明
class UHeroCombatComponent;

/**
 * 攻击状态类
 * - 支持空中或地面攻击识别
 * - 通过 CombatComponent 驱动攻击逻辑
 * - 攻击结束后通过委托或通知进入下一个状态
 */
UCLASS()
class MY2DGAMEDESIGN_API UAttackingState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;
	virtual void OnExitState_Implementation() override;

	
	// --- 输入响应：攻击状态下大多数输入将被忽略或缓冲 ---
	virtual void HandleAttackInput_Implementation() override;
	virtual void HandleJumpInputPressed_Implementation() override;
	virtual void HandleDashInput_Implementation() override;
	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override;
	
	virtual void HandleAttackEnd_Implementation() override;

	// --- 落地 / 掉落 ---
	virtual void HandleLanded_Implementation(const FHitResult& Hit) override;
	virtual void HandleWalkingOffLedge_Implementation() override;

protected:
	/** 缓存战斗组件指针 */
	UPROPERTY()
	TWeakObjectPtr<UHeroCombatComponent> CombatComp;

	/** 当前是否为空中攻击 */
	bool bIsAirAttack = false;

	/** 攻击超时备用机制（防止 Notify 丢失导致卡死） */
	FTimerHandle AttackTimeoutHandle;
	void OnAttackTimeout();

private:
	/** 攻击结束后根据是否在空中决定跳转状态 */
	void TransitionToIdleOrFalling();
};
