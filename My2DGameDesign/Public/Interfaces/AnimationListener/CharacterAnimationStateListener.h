// 文件路径: My2DGameDesign/Public/Interfaces/AnimationListener/CharacterAnimationStateListener.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Math/Vector.h" // <--- 包含 Vector
#include "CharacterAnimationStateListener.generated.h"

// 前向声明
class AActor; // <--- 前向声明 AActor

UINTERFACE(MinimalAPI, Blueprintable)
class UCharacterAnimationStateListener : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API ICharacterAnimationStateListener
{
	GENERATED_BODY()

public:
	// --- 原有方法 ---
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnIntentStateChanged(bool bNewIsWalking, bool bNewIsRunning);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnDashStateChanged(bool bNewIsDashing);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnCombatStateChanged(int32 NewComboCount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnJumpRequested();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnAirAttackStateChanged(bool bNewIsAirAttacking);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnFallingRequested();

	
	/**
	 * @brief 当角色受到伤害时调用。
	 * @param DamageAmount 受到的伤害量。
	 * @param HitDirection 伤害来源的方向（从伤害源指向角色）。
	 * @param bInterruptsCurrentAction 是否应中断当前动作（通常为 true）。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener | State")
	void OnTakeHit(float DamageAmount, const FVector& HitDirection, bool bInterruptsCurrentAction);

	/**
	 * @brief 当角色死亡时调用。
	 * @param Killer 造成致命一击的 Actor。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener | State")
	void OnDeathState(AActor* Killer);
};