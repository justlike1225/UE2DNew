// My2DGameDesign/Public/DataAssets/Enemy/EnemyMeleeAttackSettingsDA.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h" // 需要继承自 UDataAsset
#include "EnemyMeleeAttackSettingsDA.generated.h" // 生成的头文件

/**
 * UCLASS 标记此类。
 * BlueprintType 允许我们在 UE 编辑器中创建基于此 C++ 类的数据资产实例。
 */
UCLASS(BlueprintType)
class MY2DGAMEDESIGN_API UEnemyMeleeAttackSettingsDA : public UDataAsset
{
	GENERATED_BODY() // UE类宏

public:
	// --- 近战攻击核心参数 ---

	/** @brief 每次近战攻击造成的基础伤害值。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack", meta = (ClampMin = "0.0"))
	// EditAnywhere: 可在编辑器中随时修改; BlueprintReadOnly: 蓝图可读; meta: 附加信息，这里限制最小值为0
	float AttackDamage = 15.0f; // 提供一个默认值


	/** @brief 成功执行一次近战攻击后的冷却时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack", meta = (ClampMin = "0.0"))
	float AttackCooldown = 1.5f; // 提供一个默认值

	// --- 新增：攻击动画索引范围 ---
	/** @brief 可用的最小攻击动画索引 (通常从 1 开始) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack | Animation", meta = (ClampMin = "1"))
	int32 MinAttackIndex = 1;

	/** @brief 可用的最大攻击动画索引 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack | Animation", meta = (ClampMin = "1"))
	int32 MaxAttackIndex = 2; // 默认可以选 1 或 2
};
