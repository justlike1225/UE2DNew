#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HeroDashSettingsDA.generated.h"

/**
 * @brief 存储英雄冲刺能力相关配置的数据资产。
 */
UCLASS(BlueprintType) // BlueprintType 允许在蓝图中创建或使用此类型的变量
class MY2DGAMEDESIGN_API UHeroDashSettingsDA : public UDataAsset
{
	GENERATED_BODY()

public:
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash Settings") 表示此属性可以在编辑器中随时修改，并且可以在蓝图中只读访问。

	/** @brief 冲刺时施加的冲量大小。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash Settings", meta = (ClampMin = "0.0"))
	float DashSpeed = 1500.f;

	/** @brief 冲刺效果（移动覆盖）的持续时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash Settings", meta = (ClampMin = "0.01"))
	float DashDuration = 0.2f;

	/** @brief 冲刺后的冷却时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash Settings", meta = (ClampMin = "0.0"))
	float DashCooldown = 1.0f;
};