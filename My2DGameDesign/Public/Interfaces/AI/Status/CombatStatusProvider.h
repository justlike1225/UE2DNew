// 文件路径: Public/Interfaces/AI/Status/CombatStatusProvider.h (示例路径)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatStatusProvider.generated.h"

// UInterface 本身不需要包含太多东西，主要是为了让 UBT 反射系统识别
UINTERFACE(MinimalAPI, Blueprintable) // MinimalAPI 提高编译速度, Blueprintable 允许蓝图实现
class UCombatStatusProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 提供战斗相关状态查询功能的接口。
 * 行为树服务 (如 BTService_UpdateCombatStatus) 可以查询实现了此接口的 Pawn，
 * 以获取决策所需的状态信息，而无需知道 Pawn 的具体类型。
 */
class MY2DGAMEDESIGN_API ICombatStatusProvider
{
	GENERATED_BODY()

public:
	/**
	 * 检查当前是否可以执行近战攻击。
	 * @return 如果可以执行近战攻击，返回 true；否则返回 false。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Status Provider")
	bool CanPerformMeleeAttack() const;

	/**
	 * 检查当前是否可以执行传送。
	 * @return 如果可以执行传送，返回 true；否则返回 false。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Status Provider")
	bool CanPerformTeleport() const;
	/**
	 * 检查当前是否正在执行近战攻击（例如，在播放攻击动画或处于攻击状态中）。
	 * @return 如果正在执行近战攻击，返回 true；否则返回 false。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Status Provider")
	bool IsPerformingMeleeAttack() const;
	/**
	* 检查当前是否正在执行传送（例如，在播放传送动画或处于传送状态中）。
	* @return 如果正在传送，返回 true；否则返回 false。
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Status Provider")
	bool IsPerformingTeleport() const;
};