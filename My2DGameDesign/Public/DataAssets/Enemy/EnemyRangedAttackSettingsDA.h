// My2DGameDesign/Public/DataAssets/Enemy/EnemyRangedAttackSettingsDA.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyRangedAttackSettingsDA.generated.h"

// --- 前向声明 ---
// 我们需要告诉编译器 AEnemyProjectileBase 这个类存在，即使我们还没创建它
// 或者我们可以暂时使用 AActor，等创建了 AEnemyProjectileBase 再改过来。
// 为了更准确，我们先假设即将创建 AEnemyProjectileBase。
class AEnemyProjectileBase;

/**
 * UCLASS 标记此类。
 * BlueprintType 允许我们在 UE 编辑器中创建基于此 C++ 类的数据资产实例。
 */
UCLASS(BlueprintType)
class MY2DGAMEDESIGN_API UEnemyRangedAttackSettingsDA : public UDataAsset
{
	GENERATED_BODY() // UE类宏

public:
	// --- 投掷物 (Projectile) 相关 ---

	/**
	 * @brief 指定此远程攻击要生成的投掷物 Actor 的蓝图类。
	 * TSubclassOf<> 是一种特殊的属性类型，用于在编辑器中选择一个基于指定基类的类。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | Projectile")
	TSubclassOf<AEnemyProjectileBase> ProjectileClass; // 引用投掷物类

	/** @brief 投掷物生成时的初始速度。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | Projectile", meta = (ClampMin = "0.1"))
	float ProjectileSpeed = 1000.0f; // 默认值

	/** @brief 每个投掷物命中时造成的基础伤害值。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | Projectile", meta = (ClampMin = "0.0"))
	float AttackDamage = 10.0f; // 默认值

	/** @brief 投掷物的最大生存时间（秒），超过时间会自行销毁。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | Projectile", meta = (ClampMin = "0.1"))
	float ProjectileLifeSpan = 3.0f; // 默认值

	/**
	 * @brief 投掷物相对于敌人（或其某个骨骼/插槽）的生成位置偏移量。
	 * 这个偏移量会受到敌人朝向的影响。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | Projectile")
	FVector SpawnOffset = FVector(50.0f, 0.0f, 0.0f); // 默认在前方50单位处

	// --- AI 与 冷却 (Cooldown) ---

    /**
     * @brief AI 使用此攻击的最小距离。
     * 如果目标距离小于这个值，AI 可能倾向于使用近战或其他行为。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | AI", meta = (ClampMin = "0.0"))
    float AttackRangeMin = 300.0f; // 默认最小距离

    /**
     * @brief AI 使用此攻击的最大距离。
     * 目标超过这个距离，AI 就不会尝试使用此远程攻击。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | AI", meta = (ClampMin = "0.0"))
    float AttackRangeMax = 1200.0f; // 默认最大距离

	/** @brief 成功执行一次远程攻击后的冷却时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | AI", meta = (ClampMin = "0.0"))
	float AttackCooldown = 2.0f; // 默认值

	// --- (可选) 效果与表现 ---

    /** @brief 发射投掷物时在发射点播放的粒子效果 (枪口火焰/魔法效果等) (可选)。 */
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | Effects")
	// TObjectPtr<class UParticleSystem> MuzzleEffect;

	/** @brief 发射投掷物时播放的声音 (可选)。 */
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged Attack | Effects")
	// TObjectPtr<class USoundCue> FireSound;

    // --- 你可以根据需要添加更多配置项 ---
    // 例如：
    // int32 BurstCount = 1; // 一次攻击发射多少个投掷物（连发）
    // float BurstInterval = 0.1f; // 连发间隔
    // float SpreadAngle = 0.0f; // 散射角度
};