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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack", meta = (ClampMin = "0.0")) // EditAnywhere: 可在编辑器中随时修改; BlueprintReadOnly: 蓝图可读; meta: 附加信息，这里限制最小值为0
	float AttackDamage = 15.0f; // 提供一个默认值

	/**
	 * @brief 攻击范围（半径或半高）。
	 * 主要用于 AI 判断是否进入了可以发动近战攻击的距离。
	 * 实际的伤害判定范围通常由动画通知 (AnimNotify) 触发时的碰撞检测逻辑决定。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack", meta = (ClampMin = "1.0"))
	float AttackRange = 50.0f; // 提供一个默认值 (单位：厘米/Unreal Unit)

	/** @brief 成功执行一次近战攻击后的冷却时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack", meta = (ClampMin = "0.0"))
	float AttackCooldown = 1.5f; // 提供一个默认值

	
};