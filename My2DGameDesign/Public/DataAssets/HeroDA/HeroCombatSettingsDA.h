#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HeroCombatSettingsDA.generated.h"

// 前向声明
class ASwordBeamProjectile;

/**
 * @brief 存储英雄战斗能力相关配置的数据资产。
 */
UCLASS(BlueprintType)
class MY2DGAMEDESIGN_API UHeroCombatSettingsDA : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- 地面连击设置 (Ground Combo) ---

	/** @brief 连击窗口关闭后，延迟多久重置连击状态（秒）。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground Combo | Timings", meta = (ClampMin = "0.0"))
	float ComboResetDelayAfterWindowClose = 0.05f;

	/** @brief 完成一套地面连击后的攻击冷却时间（秒）。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground Combo | Timings", meta = (ClampMin = "0.0"))
	float GroundAttackCooldownDuration = 0.8f; // 重命名以区分

	/** @brief 地面每次攻击命中的基础伤害值。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground Combo | Damage", meta = (ClampMin = "0.0"))
	float GroundBaseAttackDamage = 20.0f; // 重命名以区分

	/** @brief 允许的最大地面连击次数。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground Combo | Combo", meta = (ClampMin = "1"))
	int32 MaxGroundComboCount = 3; // 重命名以区分

	// --- 空中攻击设置 (Air Attack) ---

	/** @brief 空中近身攻击的伤害值。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Attack | Melee", meta = (ClampMin = "0.0"))
	float AirAttackMeleeDamage = 15.0f;

	/** @brief 空中攻击的冷却时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Attack | Timings", meta = (ClampMin = "0.0"))
	float AirAttackCooldownDuration = 0.6f;

	// --- 剑气设置 (Sword Beam) ---

	/** @brief 空中攻击发射的剑气蓝图类。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Attack | Sword Beam")
	TSubclassOf<ASwordBeamProjectile> SwordBeamClass; // 保持 TSubclassOf

	/** @brief 剑气相对于角色Sprite的生成位置偏移。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Attack | Sword Beam")
	FVector SwordBeamSpawnOffset = FVector(50.0f, 0.0f, 0.0f);

	/** @brief 剑气抛射物的初始速度。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Attack | Sword Beam", meta = (ClampMin = "0.0"))
	float SwordBeamInitialSpeed = 1200.0f;

	/** @brief 剑气抛射物的基础伤害值。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Attack | Sword Beam", meta = (ClampMin = "0.0"))
	float SwordBeamDamage = 10.0f;

	/** @brief 剑气抛射物的生存时间（秒）。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Attack | Sword Beam", meta = (ClampMin = "0.1"))
	float SwordBeamLifeSpan = 2.0f;

	// 你可以根据需要添加更多战斗相关的配置
};
