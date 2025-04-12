// 文件路径: Public/Interfaces/AnimationListener/IEnemyAnimationEventHandler.h

#pragma once // 防止头文件重复包含

#include "CoreMinimal.h"       // UE4/5 核心功能
#include "UObject/Interface.h" // 实现 UE 接口需要包含这个
#include "EnemyAnimationEventHandler.generated.h" // UE 生成的代码

// --- UInterface 宏定义 ---
// 这是接口的元数据部分，主要给 UE 的反射系统使用。
// MinimalAPI: 减少编译依赖；Blueprintable: 允许蓝图实现或调用此接口。
UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyAnimationEventHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 敌人动画事件处理器接口
 *
 * 定义了敌人角色需要响应的关键动画事件。
 * 动画通知 (AnimNotify) 将调用这些接口函数，
 * 而具体的敌人 Actor (如 AEvilCreature) 将实现这些函数，
 * 并将事件分发给对应的组件去处理。
 */
class MY2DGAMEDESIGN_API IEnemyAnimationEventHandler
{
	GENERATED_BODY() // UE 接口类体宏

public:
	/**
	 * @brief 处理来自动画通知的“激活近战碰撞体”事件。
	 * @param ShapeIdentifier 要激活的碰撞体的名称标识符。
	 * @param Duration 碰撞体保持激活状态的持续时间（秒）。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Events | Combat")
	void HandleAnim_ActivateMeleeCollision(FName ShapeIdentifier, float Duration);

	/**
	 * @brief 处理来自动画通知的“完成传送状态”事件。
	 * 通常在传送结束动画的某个时间点调用。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Events | Ability")
	void HandleAnim_FinishTeleportState();

	/**
	 * @brief 处理来自动画通知的“重置近战攻击状态”事件。
	 * 通常在近战攻击动画序列结束后调用，用于清理攻击状态。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Events | Combat")
	void HandleAnim_ResetMeleeState();

	// --- 未来可扩展 ---
	// 如果有其他需要从动画通知触发的通用敌人事件，可以在这里添加更多函数声明。
	// 例如：
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Events | FX")
	// void HandleAnim_SpawnFootstepEffect();
	//
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Events | Sound")
	// void HandleAnim_PlayVoiceSound(USoundCue* VoiceSound);
};