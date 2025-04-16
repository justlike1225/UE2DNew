// 文件: Public/States/HeroStateBase.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InputActionValue.h"
#include "Interfaces/Context/HeroStateContext.h" // <--- 包含新的接口头文件
#include "HeroStateBase.generated.h"

// 前向声明 (保持不变)
class APaperZDCharacter_SpriteHero; // 仍然可以保留，或者完全移除对具体类的依赖
class UCharacterMovementComponent;
class ICharacterAnimationStateListener;
template <class InterfaceType> class TScriptInterface;
struct FHitResult; // <--- 添加 FHitResult 的前向声明

UCLASS(Abstract, Blueprintable)
class MY2DGAMEDESIGN_API UHeroStateBase : public UObject
{
	GENERATED_BODY()

public:
	// 修改 InitState 参数类型
	virtual void InitState(TScriptInterface<IHeroStateContext> InHeroContext); // <--- 修改参数类型

	// --- 状态生命周期 ---
	UFUNCTION(BlueprintNativeEvent, Category = "State Lifecycle")
	void OnEnterState();
	virtual void OnEnterState_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Lifecycle")
	void OnExitState();
	virtual void OnExitState_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Lifecycle")
	void TickState(float DeltaTime);
	virtual void TickState_Implementation(float DeltaTime);

	// --- 输入处理 ---
	UFUNCTION(BlueprintNativeEvent, Category = "State Input Handling")
	void HandleMoveInput(const FInputActionValue& Value);
	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, Category = "State Input Handling")
	void HandleJumpInputPressed();
	virtual void HandleJumpInputPressed_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Input Handling")
	void HandleJumpInputReleased();
	virtual void HandleJumpInputReleased_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Input Handling")
	void HandleAttackInput();
	virtual void HandleAttackInput_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Input Handling")
	void HandleDashInput();
	virtual void HandleDashInput_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Input Handling")
	void HandleRunInputPressed();
	virtual void HandleRunInputPressed_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Input Handling")
	void HandleRunInputReleased();
	virtual void HandleRunInputReleased_Implementation();

	// --- 事件处理 ---
	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleLanded(const FHitResult& Hit);
	virtual void HandleLanded_Implementation(const FHitResult& Hit);

	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleWalkingOffLedge();
	virtual void HandleWalkingOffLedge_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleTakeDamage();
	virtual void HandleTakeDamage_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleHurtRecovery();
	virtual void HandleHurtRecovery_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleDeath();
	virtual void HandleDeath_Implementation();

	// --- 状态结束事件 (由 AnimNotify 或组件回调触发) ---
	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleAttackEnd(); // 由 CombatComponent 通过 AnimNotify -> Character -> State 调用
	virtual void HandleAttackEnd_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleDashEnd(); // 由 DashComponent 通过 Delegate -> Character -> State 调用
	virtual void HandleDashEnd_Implementation();

	/** 尝试切换到新状态 (内部使用，通过接口调用) */
	virtual void TrySetState(TSubclassOf<UHeroStateBase> NewStateClass); // <--- 保持 virtual，但实现会改变

protected:
	// 指向拥有此状态的上下文 (通过接口)
	UPROPERTY(BlueprintReadOnly, Category = "State Context")
	TScriptInterface<IHeroStateContext> HeroContext; // <--- 修改类型

	// 方便访问移动组件 (在 InitState 中通过接口获取)
	UPROPERTY(BlueprintReadOnly, Category = "State Context")
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

	// 方便访问动画监听器 (在 InitState 中通过接口获取)
	UPROPERTY(BlueprintReadOnly, Category = "State Context")
	TScriptInterface<ICharacterAnimationStateListener> AnimListener;

    // 方便访问生命组件 (在 InitState 中通过接口获取)
	UPROPERTY(BlueprintReadOnly, Category = "State Context")
	TWeakObjectPtr<UHealthComponent> HealthComponent; // <--- 添加
};