// 文件: Public/States/HeroStateBase.h (你需要创建 States 目录和这个头文件)
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InputActionValue.h" // 可能需要处理输入值
#include "HeroStateBase.generated.h"

// 前向声明
class APaperZDCharacter_SpriteHero;
class UCharacterMovementComponent;
class ICharacterAnimationStateListener; // 前向声明动画监听器接口
template <class InterfaceType>
class TScriptInterface; // 前向声明 TScriptInterface

/**
 * 英雄角色状态的抽象基类
 */
UCLASS(Abstract, Blueprintable) // Abstract: 不能直接实例化; Blueprintable: 允许蓝图继承（可选）
class MY2DGAMEDESIGN_API UHeroStateBase : public UObject
{
	GENERATED_BODY()

public:
	// 初始化状态，传入所属英雄
	virtual void InitState(APaperZDCharacter_SpriteHero* InHeroContext);
	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleAttackEnd();
	virtual void HandleAttackEnd_Implementation();

	/** 处理冲刺结束事件 (通常由 DashComponent 触发) */
	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleDashEnd();
	virtual void HandleDashEnd_Implementation();
	// --- 状态生命周期 ---

	/** 当进入此状态时调用 */
	UFUNCTION(BlueprintNativeEvent, Category = "State Lifecycle")
	void OnEnterState();
	virtual void OnEnterState_Implementation(); // C++ 默认实现

	/** 当退出此状态时调用 */
	UFUNCTION(BlueprintNativeEvent, Category = "State Lifecycle")
	void OnExitState();
	virtual void OnExitState_Implementation(); // C++ 默认实现

	/** 如果状态需要每帧更新，则调用 */
	UFUNCTION(BlueprintNativeEvent, Category = "State Lifecycle")
	void TickState(float DeltaTime);
	virtual void TickState_Implementation(float DeltaTime);


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

	/** 处理着陆事件 */
	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleLanded(const FHitResult& Hit);
	virtual void HandleLanded_Implementation(const FHitResult& Hit);

	/** 处理从边缘掉落事件 */
	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleWalkingOffLedge();
	virtual void HandleWalkingOffLedge_Implementation();

	/** 处理受击事件 - 注意：这里可能只用于触发状态切换，具体伤害计算仍在Hero类 */
	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleTakeDamage(); // 简化参数，状态只关心是否要切换到Hurt
	virtual void HandleTakeDamage_Implementation();

	/** 处理硬直恢复事件 */
	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleHurtRecovery();
	virtual void HandleHurtRecovery_Implementation();
	

	/** 处理死亡事件 */
	UFUNCTION(BlueprintNativeEvent, Category = "State Event Handling")
	void HandleDeath();
	virtual void HandleDeath_Implementation();


	/** 尝试切换到新状态 */
	UFUNCTION(BlueprintCallable, Category = "State Management")
	virtual void TrySetState(TSubclassOf<UHeroStateBase> NewStateClass);

protected:
	// 指向拥有此状态的英雄角色 (上下文)
	UPROPERTY(BlueprintReadOnly, Category = "State Context")
	TObjectPtr<APaperZDCharacter_SpriteHero> HeroContext;

	// 方便访问移动组件 (在 InitState 中设置)
	UPROPERTY(BlueprintReadOnly, Category = "State Context")
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent; // 使用弱指针更安全

	// 方便访问动画监听器 (在 InitState 中设置)
	UPROPERTY(BlueprintReadOnly, Category = "State Context")
	TScriptInterface<ICharacterAnimationStateListener> AnimListener;
};
