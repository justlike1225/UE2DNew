// My2DGameDesign/Public/Components/EnemyMeleeAttackComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h" // <-- 包含 TimerManager
#include "Containers/Set.h" // <-- 包含 TSet
#include "EnemyMeleeAttackComponent.generated.h"

// --- 前向声明 ---
class UEnemyMeleeAttackSettingsDA;
class AEnemyCharacterBase;
class AActor;
class IEnemyMeleeAttackAnimListener; // 需要监听器接口
template<class InterfaceType> class TScriptInterface; // 需要 TScriptInterface

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MY2DGAMEDESIGN_API UEnemyMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyMeleeAttackComponent();

	/**
	 * @brief 由 AI 或其他系统调用以执行一次近战攻击。
	 * @param Target (可选参数，现在主要用于朝向或AI决策，不再直接用于伤害) 要攻击的目标 Actor。
	 * @return 如果成功开始攻击（不在冷却中且不在攻击中），则返回 true。
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee")
	bool ExecuteAttack(AActor* Target = nullptr); // Target 变为可选

	/** 查询当前是否可以执行攻击 (主要检查冷却时间) */
	UFUNCTION(BlueprintPure, Category = "Enemy Attack | Melee | Status")
	bool CanAttack() const { return bCanAttack; }

	/** 查询当前是否正在执行攻击动画/逻辑中 */
    UFUNCTION(BlueprintPure, Category = "Enemy Attack | Melee | Status")
    bool IsAttacking() const { return bIsAttacking; }

    // --- 由 AnimNotify 调用 ---
    /**
     * @brief 在攻击动画的伤害判定开始时调用，激活对应的碰撞体。
     * @param ShapeIdentifier 要激活的碰撞体的名称 (来自 AEvilCreature::EvilCreatureAttackShapeNames)。
     * @param Duration 碰撞体保持激活状态的持续时间（秒）。
     */
    UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee | AnimNotify")
    void ActivateMeleeCollision(FName ShapeIdentifier, float Duration = 0.1f); // 提供默认持续时间

    /**
     * @brief 在攻击动画的伤害判定结束时（或由计时器）调用，停用对应的碰撞体。
     * @param ShapeIdentifier 要停用的碰撞体的名称。
     */
    UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee | AnimNotify") // 也可以由计时器调用
    void DeactivateMeleeCollision(FName ShapeIdentifier);


    // --- 公开的数据资产引用，方便在 AEvilCreature 中访问 ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Configuration")
	TObjectPtr<UEnemyMeleeAttackSettingsDA> AttackSettings;

    // --- 公开的命中记录集合，方便在 AEvilCreature 的 HandleMeleeHit 中访问和修改 ---
    UPROPERTY(Transient) // 瞬态，不需要保存
    TSet<TObjectPtr<AActor>> HitActorsThisSwing;


protected:
	// 生命周期函数
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- 内部状态 ---
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Status", meta=(AllowPrivateAccess="true"))
	bool bCanAttack = true;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Status", meta=(AllowPrivateAccess="true"))
    bool bIsAttacking = false; // 标记整个攻击流程是否在进行中

    // 移除 CurrentTarget (不再依赖它施加伤害)
    // TWeakObjectPtr<AActor> CurrentTarget;

    // 移除 bHasAppliedDamageThisAttack (逻辑移到 HitActorsThisSwing)

	/** 攻击冷却计时器的句柄 */
	FTimerHandle AttackCooldownTimer;
    /** 当前激活的攻击碰撞体的关闭计时器句柄 */
    FTimerHandle ActiveCollisionTimerHandle;
    /** 当前激活的碰撞体的名称 (用于关闭) */
    FName ActiveCollisionShapeName;


	// --- 内部引用 ---
	UPROPERTY(Transient)
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;


private:
	// --- 内部辅助函数 ---
	/** 启动攻击冷却计时器 */
	void StartAttackCooldown();

	/** 当攻击冷却计时器结束时调用 */
	UFUNCTION()
	void OnAttackCooldownFinished();

    /** 获取动画监听器接口 */
    TScriptInterface<IEnemyMeleeAttackAnimListener> GetAnimListener() const;

    /** 开始一次攻击挥砍时调用（清空命中记录）*/
    void BeginAttackSwing();
};