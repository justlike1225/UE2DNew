// My2DGameDesign/Public/Interfaces/EnemyAnimationStateListener.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyAnimationStateListener.generated.h" // 生成的头文件

// --- 前向声明 ---
// 如果接口函数需要传递特定的枚举或类作为参数，可以在这里前向声明
// 例如: enum class EEnemyAttackType : uint8;
// 例如: class UEnemyAbility;

// UInterface boilerplate
UINTERFACE(MinimalAPI, Blueprintable) // 同样允许蓝图实现
class UEnemyAnimationStateListener : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief C++ 接口，用于敌人动画实例 (AnimInstance) 接收来自 AI 或组件的状态更新通知。
 * 实现此接口的通常是 UEnemyAnimInstanceBase 或其子类。
 */
class MY2DGAMEDESIGN_API IEnemyAnimationStateListener
{
	GENERATED_BODY()

public:
	/**
	 * @brief 当敌人的移动相关状态发生变化时调用。
	 * @param Speed 敌人当前的移动速度大小 (通常是水平速度)。
	 * @param bIsFalling 敌人当前是否处于下落状态。
	 * @param bIsMoving 敌人当前是否有移动意图或正在移动。
	 * (根据你的具体实现，这个参数可以代表不同的含义，例如AI是否正在执行MoveTo任务)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Movement")
	void OnMovementStateChanged(float Speed, bool bIsFalling, bool bIsMoving);

	/**
	 * @brief 当敌人开始执行近战攻击动作时调用。
	 * @param Target (可选) 攻击的目标 Actor，动画实例可能需要根据目标做一些特殊处理。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Combat")
	void OnMeleeAttackStarted(AActor* Target);

    /**
	 * @brief 当敌人开始执行远程攻击动作时调用。
	 * @param Target (可选) 攻击的目标 Actor。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Combat")
	void OnRangedAttackStarted(AActor* Target);

    /**
     * @brief 当敌人的攻击动作结束或中断时调用 (可选)。
     * 可以用于将动画实例的攻击状态变量重置。
     * 或者，攻击状态的重置也可以在动画本身结束时通过 AnimNotify 来处理。
     */
    // UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Combat")
    // void OnAttackEnded();

    /**
     * @brief 当敌人进入或离开眩晕状态时调用 (示例)。
     * @param bIsStunned 是否进入眩晕状态。
     */
    // UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | State")
    // void OnStunStateChanged(bool bIsStunned);

    /**
     * @brief 当敌人死亡时调用。
     * 这通常用于触发死亡动画。
     * @param Killer (可选) 杀死敌人的 Actor。
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | State")
    void OnDeathState(AActor* Killer);

	// --- 你可以根据敌人所需的状态，自由地在这里添加更多的通知函数 ---
	// 例如：
	// OnSpecialAbilityStarted(FName AbilityName);
	// OnBlockingStateChanged(bool bIsBlocking);
    // OnAlertnessStateChanged(EEnemyAlertness State);
};