// My2DGameDesign/Public/Components/EnemyMeleeAttackComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyMeleeAttackComponent.generated.h"


// --- 前向声明 ---
class UEnemyMeleeAttackSettingsDA; // 我们的数据资产
class AEnemyCharacterBase;         // 敌人角色基类
class AActor;                     // Actor 基类
class FTimerManager;              // 需要 TimerManager 来处理定时器句柄

/**
 * 负责处理敌人近战攻击逻辑的组件。
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MY2DGAMEDESIGN_API UEnemyMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 构造函数
	UEnemyMeleeAttackComponent();

	/**
	 * @brief 由 AI 或其他系统调用以执行一次近战攻击。
	 * @param Target 要攻击的目标 Actor。
	 * @return 如果成功开始攻击（不在冷却中且不在攻击中），则返回 true。
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee")
	bool ExecuteAttack(AActor* Target);

	/**
	 * @brief 由动画通知 (AnimNotify) 在攻击动画的伤害判定帧调用。
	 * 负责实际对目标应用伤害。
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee | AnimNotify")
	void HandleDamageApplication();

	/** 查询当前是否可以执行攻击 (主要检查冷却时间) */
	UFUNCTION(BlueprintPure, Category = "Enemy Attack | Melee | Status")
	bool CanAttack() const { return bCanAttack; }

	/** 查询当前是否正在执行攻击动画/逻辑中 */
    UFUNCTION(BlueprintPure, Category = "Enemy Attack | Melee | Status")
    bool IsAttacking() const { return bIsAttacking; }


protected:
	// 生命周期函数
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * @brief 指向配置此攻击的数据资产。
	 * 需要在蓝图或 C++ 中为使用此组件的敌人配置这个属性。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Configuration")
	TObjectPtr<UEnemyMeleeAttackSettingsDA> AttackSettings;

	// --- 内部状态 ---

	/** 标记当前是否可以攻击（冷却时间是否结束） */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Status", meta=(AllowPrivateAccess="true"))
	bool bCanAttack = true;

	/** 标记当前是否正在执行一次攻击（从 ExecuteAttack 开始到冷却结束）*/
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Status", meta=(AllowPrivateAccess="true"))
    bool bIsAttacking = false;

	/** 存储当前攻击的目标，以便 AnimNotify 函数知道对谁造成伤害 */
    UPROPERTY(VisibleInstanceOnly, Transient, Category = "Enemy Attack | Melee | Status", meta=(AllowPrivateAccess="true"))
    TWeakObjectPtr<AActor> CurrentTarget; // 使用弱指针，防止目标被销毁后指针悬空

	/** 攻击冷却计时器的句柄 */
	FTimerHandle AttackCooldownTimer;

    /** 标记在本次攻击动画中是否已经施加了伤害 (防止 AnimNotify 重复触发伤害) */
    bool bHasAppliedDamageThisAttack = false;


	// --- 内部引用 ---

	/** 指向拥有此组件的敌人角色的弱指针 */
	UPROPERTY(Transient)
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;

	

private:
	// --- 内部辅助函数 ---	

	/** 启动攻击冷却计时器 */
	void StartAttackCooldown();

	/** 当攻击冷却计时器结束时调用 */
	UFUNCTION() // UFUNCTION 宏是定时器回调函数所必需的
	void OnAttackCooldownFinished();

};