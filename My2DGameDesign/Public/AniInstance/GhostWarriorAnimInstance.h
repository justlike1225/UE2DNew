#pragma once

#include "CoreMinimal.h"
#include "AniInstance/EnemyAnimInstanceBase.h"
// --- 包含需要的监听器接口 ---
#include "Interfaces/AnimationListener/EnemyMeleeAttackAnimListener.h"
#include "Interfaces/AnimationListener/MeleeStateResetListener.h"
#include "GhostWarriorAnimInstance.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UGhostWarriorAnimInstance : public UEnemyAnimInstanceBase,
													  public IEnemyMeleeAttackAnimListener, // 实现近战攻击开始监听
													  public IMeleeStateResetListener    // 实现近战状态重置监听
{
	GENERATED_BODY()

protected:
	// --- 动画状态变量 (供 PaperZD 动画蓝图使用) ---

	/** 是否正在执行近战攻击 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAttackingMelee = false;

	/** 当前执行的近战攻击索引 (例如 1 或 2，对应 Attack_1, Attack_2) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Combat", meta = (AllowPrivateAccess = "true"))
	int32 MeleeAttackIndex = 0;
	

	// IEnemyMeleeAttackAnimListener
	virtual void OnMeleeAttackStarted_Implementation(AActor* Target, int32 AttackIndex) override;

	// IMeleeStateResetListener
	virtual void HandleMeleeAttackEnd_Implementation() override;

public:
	// (可选) 如果需要在动画蓝图之外直接访问这些状态，可以提供 Getter 函数
	UFUNCTION(BlueprintPure, Category = "Enemy State | Combat")
	bool IsAttackingMelee() const { return bIsAttackingMelee; }

	UFUNCTION(BlueprintPure, Category = "Enemy State | Combat")
	int32 GetMeleeAttackIndex() const { return MeleeAttackIndex; }
};