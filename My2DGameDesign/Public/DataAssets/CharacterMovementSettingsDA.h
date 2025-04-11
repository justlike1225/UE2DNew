// 文件路径: My2DGameDesign/Public/DataAssets/CharacterMovementSettingsDA.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterMovementSettingsDA.generated.h"

/**
 * 数据资产，用于存储角色的通用运动属性配置。
 * 这些值将在角色初始化时应用到其 CharacterMovementComponent 上。
 */
UCLASS(BlueprintType) // BlueprintType 允许在编辑器中创建此类型的资产实例
class MY2DGAMEDESIGN_API UCharacterMovementSettingsDA : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- 地面移动 ---

	/** @brief 最大行走速度 (单位: cm/s)。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Ground", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	float MaxWalkSpeed = 200.0f;

	/** @brief 最大奔跑速度 (单位: cm/s)，如果角色支持奔跑。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Ground", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	float MaxRunSpeed = 500.0f; // 添加奔跑速度

	/** @brief 行走时的最大加速度 (单位: cm/s^2)。控制达到最大速度的快慢。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Ground", meta = (ClampMin = "0.0", ForceUnits = "cm/s2"))
	float MaxAcceleration = 2048.0f;

	/** @brief 地面摩擦力。数值越高，停止越快。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Ground", meta = (ClampMin = "0.0"))
	float GroundFriction = 8.0f;

	/** @brief 行走时的制动减速度 (单位: cm/s^2)。松开移动键时应用的减速度。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Ground", meta = (ClampMin = "0.0", ForceUnits = "cm/s2"))
	float BrakingDecelerationWalking = 2048.0f;

	// --- 空中移动 ---

	/** @brief 跳跃时的垂直初速度 (Z轴) (单位: cm/s)。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Air", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	float JumpZVelocity = 420.0f;

	/** @brief 空中控制力。0 表示无空中控制，1 表示完全控制。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Air", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AirControl = 0.2f;

	/** @brief 重力缩放。1 是正常重力。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Air", meta = (ClampMin = "0.0"))
	float GravityScale = 1.0f;

	// --- 其他 ---
	// 你可以在这里添加更多来自 UCharacterMovementComponent 的属性，例如:
	// MaxFlySpeed, MaxSwimSpeed, BrakingFrictionFactor, RotationRate 等等，根据你的需要。
};