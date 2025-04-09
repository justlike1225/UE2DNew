// My2DGameDesign/Public/DataAssets/Enemy/TeleportSettingsDA.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TeleportSettingsDA.generated.h"

// 前向声明 (如果需要引用粒子、声音等)
// class UParticleSystem;
// class USoundCue;

UCLASS(BlueprintType)
class MY2DGAMEDESIGN_API UTeleportSettingsDA : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief 传送的最大距离。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport", meta = (ClampMin = "0.0"))
	float TeleportMaxDistance = 800.0f;

	/** @brief 传送的最小距离 (避免原地传送)。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport", meta = (ClampMin = "0.0"))
	float TeleportMinDistance = 100.0f;

	/** @brief 传送的冷却时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport", meta = (ClampMin = "0.0"))
	float TeleportCooldown = 5.0f;

	/** @brief (可选) 传送开始时的粒子效果。 */
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport | Effects")
	// TObjectPtr<UParticleSystem> TeleportStartEffect;

	/** @brief (可选) 传送结束时的粒子效果。 */
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport | Effects")
	// TObjectPtr<UParticleSystem> TeleportEndEffect;

	/** @brief (可选) 传送开始时的声音。 */
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport | Effects")
	// TObjectPtr<USoundCue> TeleportStartSound;

	/** @brief (可选) 传送结束时的声音。 */
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport | Effects")
	// TObjectPtr<USoundCue> TeleportEndSound;

	/** @brief 传送实际执行前的准备时间（秒），用于播放动画或特效。如果为0，则立即传送。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport", meta = (ClampMin = "0.0"))
	float TeleportCastTime = 0.5f;
};