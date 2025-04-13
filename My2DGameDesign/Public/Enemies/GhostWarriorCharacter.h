#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyCharacterBase.h"
// --- 包含所需接口的头文件 ---
#include "Interfaces/MeleeShapeProvider.h"
#include "Interfaces/AI/Abilities/MeleeAbilityExecutor.h"
#include "Interfaces/AI/Status/CombatStatusProvider.h"
#include "Interfaces/AnimationEvents/EnemyAnimationEventHandler.h"
#include "GhostWarriorCharacter.generated.h" // 要放在最后 include

class UEnemyMeleeAttackSettingsDA;
// --- 前向声明所需组件 ---
class UEnemyMeleeAttackComponent;
class UCapsuleComponent; // 或者 UBoxComponent，取决于你的攻击形状

// --- (可选但推荐) 为这个敌人的攻击形状定义一个命名空间 ---
namespace GhostWarriorAttackShapeNames
{
	// 如果有多个攻击动作对应不同碰撞体，可以在这里定义
	const FName SwordSlash(TEXT("SwordSlash")); // 假设一个主要的剑击碰撞体
}

UCLASS()
class MY2DGAMEDESIGN_API AGhostWarriorCharacter : public AEnemyCharacterBase,
                                                   public IMeleeShapeProvider,           // 实现: 提供近战碰撞体
                                                   public ICombatStatusProvider,       // 实现: 允许 AI 查询状态 (能否攻击，是否在攻击等)
                                                   public IMeleeAbilityExecutor,       // 实现: 允许 AI 命令执行近战攻击
                                                   public IEnemyAnimationEventHandler // 实现: 处理动画通知事件
{
	GENERATED_BODY()

public:
	AGhostWarriorCharacter(); // 构造函数

	// --- (可选) 组件访问器 ---
	UFUNCTION(BlueprintPure, Category = "Components | Combat")
	UEnemyMeleeAttackComponent* GetMeleeAttackComponent() const { return MeleeAttackComponent; }

	// --- 接口实现声明 (Override) ---

	// IMeleeShapeProvider
	virtual UPrimitiveComponent* GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const override;

	// ICombatStatusProvider
	virtual bool CanPerformMeleeAttack_Implementation() const override;
	virtual bool CanPerformTeleport_Implementation() const override;
	virtual bool IsPerformingMeleeAttack_Implementation() const override;
	virtual bool IsPerformingTeleport_Implementation() const override; 
	virtual bool ExecuteMeleeAttack_Implementation(EEnemyMeleeAttackType AttackType, AActor* Target) override;
	// 你可能还需要添加 CanPerformRangedAttack 等，都返回 false



	// IEnemyAnimationEventHandler
	virtual void HandleAnim_ActivateMeleeCollision_Implementation(FName ShapeIdentifier, float Duration) override;
	virtual void HandleAnim_FinishTeleportState_Implementation() override; // 必须实现，即使不做事
	virtual void HandleAnim_ResetMeleeState_Implementation() override;

protected:
    // 重写生命周期函数以附加碰撞体
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
	
	
	
	// --- 组件声明 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnemyMeleeAttackComponent> MeleeAttackComponent;

	// 声明用于近战攻击的碰撞体 (这里用胶囊体举例)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> AttackShape;

	

};