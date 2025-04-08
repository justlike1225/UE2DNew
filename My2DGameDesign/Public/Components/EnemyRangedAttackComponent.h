// My2DGameDesign/Public/Components/EnemyRangedAttackComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyRangedAttackComponent.generated.h"

// --- 前向声明 ---
class UEnemyRangedAttackSettingsDA; // 远程攻击数据资产
class AEnemyCharacterBase;          // 敌人角色基类
class AActor;                      // Actor 基类
class AEnemyProjectileBase;       // 敌人投掷物基类
class FTimerManager;               // 定时器管理器

/**
 * 负责处理敌人远程投掷物攻击逻辑的组件。
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MY2DGAMEDESIGN_API UEnemyRangedAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 构造函数
	UEnemyRangedAttackComponent();

	/**
	 * @brief 由 AI 或其他系统调用以执行一次远程攻击。
	 * @param Target 要攻击的目标 Actor。
	 * @return 如果成功开始攻击（不在冷却中、不在攻击中且配置有效），则返回 true。
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Ranged")
	bool ExecuteAttack(AActor* Target);

	/**
	 * @brief 由动画通知 (AnimNotify) 在攻击动画的投掷物发射帧调用。
	 * 负责实际生成并初始化投掷物。
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Ranged | AnimNotify")
	void HandleSpawnProjectile();

	/** 查询当前是否可以执行攻击 (主要检查冷却时间) */
	UFUNCTION(BlueprintPure, Category = "Enemy Attack | Ranged | Status")
	bool CanAttack() const { return bCanAttack; }

	/** 查询当前是否正在执行攻击动画/逻辑中 */
    UFUNCTION(BlueprintPure, Category = "Enemy Attack | Ranged | Status")
    bool IsAttacking() const { return bIsAttacking; }


protected:
	// 生命周期函数
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * @brief 指向配置此远程攻击的数据资产。
	 * 必须在蓝图或 C++ 中为使用此组件的敌人配置。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack | Ranged | Configuration")
	TObjectPtr<UEnemyRangedAttackSettingsDA> AttackSettings;

	// --- 内部状态 ---

	/** 标记当前是否可以攻击（冷却时间是否结束） */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Ranged | Status", meta=(AllowPrivateAccess="true"))
	bool bCanAttack = true;

	/** 标记当前是否正在执行一次攻击 */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Ranged | Status", meta=(AllowPrivateAccess="true"))
    bool bIsAttacking = false;

	/** 存储当前攻击的目标，以便 AnimNotify 函数知道朝哪个方向发射 */
    UPROPERTY(VisibleInstanceOnly, Transient, Category = "Enemy Attack | Ranged | Status", meta=(AllowPrivateAccess="true"))
    TWeakObjectPtr<AActor> CurrentTarget;

	/** 攻击冷却计时器的句柄 */
	FTimerHandle AttackCooldownTimer;

	// --- 内部引用 ---

	/** 指向拥有此组件的敌人角色的弱指针 */
	UPROPERTY(Transient)
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;



private:
	// --- 内部辅助函数 ---

	/** 启动攻击冷却计时器 */
	void StartAttackCooldown();

	/** 当攻击冷却计时器结束时调用 */
	UFUNCTION()
	void OnAttackCooldownFinished();
	

    /**
     * @brief 计算投掷物的生成位置和旋转。
     * @param OutSpawnLocation 输出计算好的生成位置。
     * @param OutSpawnRotation 输出计算好的生成旋转 (朝向目标)。
     * @return 如果成功计算（例如目标有效），则返回 true。
     */
    bool CalculateSpawnTransform(FVector& OutSpawnLocation, FRotator& OutSpawnRotation) const;
};