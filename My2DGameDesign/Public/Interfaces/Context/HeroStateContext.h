// 文件路径: Public/Interfaces/Context/HeroStateContext.h (如果目录不存在请创建)
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InputActionValue.h" // 需要 FInputActionValue
#include "HeroStateContext.generated.h"

// 前向声明 (Forward Declarations)
class UCharacterMovementComponent;
class ICharacterAnimationStateListener; // 动画监听器接口
class UHeroStateBase;                 // 状态基类
class UHealthComponent;               // 生命组件 (添加，因为状态需要检查死亡)
struct FHitResult;                    // 命中结果结构体
template <class InterfaceType> class TScriptInterface; // TScriptInterface 模板

// UInterface 本身，主要供引擎反射系统使用
UINTERFACE(MinimalAPI, Blueprintable)
class UHeroStateContext : public UInterface
{
	GENERATED_BODY()
};

/**
 * IHeroStateContext 接口类
 * 定义了 Hero 状态类与其上下文（通常是角色本身）交互所需的所有方法。
 * 这使得状态类可以独立于具体的角色类实现。
 */
class MY2DGAMEDESIGN_API IHeroStateContext
{
	GENERATED_BODY()

public:
	// --- 必要组件访问器 ---
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Accessors")
	UCharacterMovementComponent* GetMovementComponent() const; // 获取移动组件

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Accessors")
	TScriptInterface<ICharacterAnimationStateListener> GetAnimStateListener() const; // 获取动画状态监听器

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Accessors")
	UHealthComponent* GetHealthComponent() const; // 获取生命组件，用于检查死亡

	// --- 状态管理 ---
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | State")
	void RequestStateChange(TSubclassOf<UHeroStateBase> NewStateClass); // 请求状态切换

	// --- 角色动作 ---
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Actions")
	void PerformJump(); // 执行跳跃

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Actions")
	void PerformStopJumping(); // 停止跳跃

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Actions")
	void InterruptAction(); // 停止跳跃
   
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Actions")
	void ApplyMovementInput(const FVector& WorldDirection, float ScaleValue); // 应用移动输入

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Actions")
	void RequestAttack(); // 请求攻击 (由上下文决定如何执行)

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Actions")
	void RequestDash(); // 请求冲刺 (由上下文决定如何执行)

	// --- 属性查询 ---
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Properties")
	float GetCachedWalkSpeed() const; // 获取缓存的行走速度

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Properties")
	float GetCachedRunSpeed() const; // 获取缓存的奔跑速度

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hero Context | Properties")
	const AActor* GetOwningActor() const; // 获取上下文所属的 Actor
	
};