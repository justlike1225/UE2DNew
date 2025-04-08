// My2DGameDesign/Public/Interfaces/CharacterAnimationStateListener.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Math/Vector.h" // 需要包含 FVector
#include "CharacterAnimationStateListener.generated.h"

// UInterface boilerplate
UINTERFACE(MinimalAPI, Blueprintable)
class UCharacterAnimationStateListener : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief C++接口，用于动画实例接收来自角色或其组件的状态更新通知。
 * 实现此接口的通常是 AnimInstance 类。
 */
class MY2DGAMEDESIGN_API ICharacterAnimationStateListener
{
	GENERATED_BODY()

public:
	/**
	 * @brief 当角色的移动意图状态发生变化时调用。
	 * @param bNewIsWalking 角色当前是否在行走。
	 * @param bNewIsRunning 角色当前是否在奔跑。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnIntentStateChanged(bool bNewIsWalking, bool bNewIsRunning);

	/**
	 * @brief 当角色的冲刺状态发生变化时调用。
	 * @param bNewIsDashing 角色当前是否在冲刺。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnDashStateChanged(bool bNewIsDashing);

	/**
	 * @brief 当角色的战斗状态（例如连击数）发生变化时调用。
	 * @param NewComboCount 当前的连击数。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnCombatStateChanged(int32 NewComboCount);

	// --- 【新增】当角色请求跳跃动画时调用 ---
	/**
	 * @brief 当角色执行跳跃动作，需要动画实例响应时调用。
	 * 通常在此函数实现中调用 JumpToNode("Jump")。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnJumpRequested();


	/**
	   * @brief 当角色的空中攻击状态发生变化时调用。
	   * @param bNewIsAirAttacking 角色当前是否正在执行空中攻击。
	   */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnAirAttackStateChanged(bool bNewIsAirAttacking);

};