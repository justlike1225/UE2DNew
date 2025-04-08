// My2DGameDesign/Public/Components/HealthComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h" // 需要包含生成的头文件

// --- 委托 (Delegate) 声明 ---
// 委托是 C++ 中的一种类型安全的函数指针，常用于事件通知，是实现低耦合的重要机制。

// 当生命值变化时广播 (参数：新的当前生命值, 最大生命值)
// DYNAMIC_MULTICAST 意味着这个委托可以在蓝图中绑定事件。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

// 当生命值降为0或以下 (死亡) 时广播 (参数：造成致命一击的Actor指针)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathSignature, AActor*, Killer);


/**
 * UCLASS标记让UE的反射系统能够识别这个类。
 * ClassGroup 和 meta 是编辑器的元数据。
 * BlueprintSpawnableComponent 允许我们在蓝图中将这个组件添加到 Actor 上。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MY2DGAMEDESIGN_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY() // UE类需要的宏

public:
	// 构造函数
	UHealthComponent();

	/**
	 * @brief 公开的函数，用于对拥有此组件的 Actor 施加伤害。
	 * 这是处理伤害的主要入口点。
	 * @param DamageAmount 基础伤害量。
	 * @param DamageCauser 造成伤害的 Actor (例如玩家角色)。
	 * @param InstigatorController 造成伤害的控制器 (例如玩家控制器)。
	 * @return 返回实际造成的伤害量 (目前实现中未做修改，直接返回 DamageAmount)。
	 */
	UFUNCTION(BlueprintCallable, Category = "Health | Actions") // BlueprintCallable 允许在蓝图中调用此函数
	float TakeDamage(float DamageAmount, AActor* DamageCauser, AController* InstigatorController);

	/** 获取当前生命值 - BlueprintPure 表示在蓝图中调用它不会改变对象状态，像一个纯查询 */
	UFUNCTION(BlueprintPure, Category = "Health | Status")
	float GetCurrentHealth() const { return CurrentHealth; }

	/** 获取最大生命值 */
	UFUNCTION(BlueprintPure, Category = "Health | Status")
	float GetMaxHealth() const { return MaxHealth; }

	/** 判断是否已经死亡 */
	UFUNCTION(BlueprintPure, Category = "Health | Status")
	bool IsDead() const { return bIsDead; }

	// --- 委托 (Events) ---
	// 这些委托允许其他类（比如 AI、UI、特效管理器）监听生命值的变化和死亡事件，而不需要直接了解 HealthComponent 的内部实现。

	/** 当生命值发生变化时广播 (无论是受伤还是可能的治疗) */
	UPROPERTY(BlueprintAssignable, Category = "Health | Events") // BlueprintAssignable 允许在蓝图中绑定此事件
	FOnHealthChangedSignature OnHealthChanged;

	/** 当生命值降至0或以下时广播 */
	UPROPERTY(BlueprintAssignable, Category = "Health | Events")
	FOnDeathSignature OnDeath;

protected:
	// protected 访问说明符意味着这些成员只能被这个类自身或者它的子类访问。

	// 生命周期函数，当组件在游戏世界中开始运行时被调用。
	virtual void BeginPlay() override;

	/**
	 * UPROPERTY 宏让UE能识别这个变量。
	 * EditDefaultsOnly: 只能在类默认属性（蓝图编辑器或C++默认值）中设置，不能在关卡实例中单独修改。
	 * BlueprintReadWrite: 允许蓝图读取和写入这个默认值。
	 * Category: 在编辑器细节面板中进行分组。
	 * meta = (ClampMin = "0.1"): 限制编辑器中此值的最小值为0.1，防止设置无效生命值。
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health | Configuration", meta = (ClampMin = "0.1"))
	float DefaultMaxHealth = 100.0f; // 提供一个默认的最大生命值

	// --- 内部状态变量 ---
	// 这些变量存储组件的当前状态。

	/** 当前生命值 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health | Status", meta = (AllowPrivateAccess = "true"))
	// VisibleInstanceOnly: 只能在关卡中选中实例时看到这个值（用于调试），不能编辑。
	// BlueprintReadOnly: 蓝图只能读取，不能写入。
	// AllowPrivateAccess: 允许在类定义内部直接访问（即使它是 private 或 protected）。
	float CurrentHealth = 0.0f;

	/** 最大生命值 (运行时使用的最终值，通常由 DefaultMaxHealth 初始化) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health | Status", meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 0.0f;

	/** 标记是否已经死亡，防止重复触发死亡逻辑 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health | Status", meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

private:
	// private 访问说明符意味着这些成员只能被这个类自身访问。

    /** 内部辅助函数，用于处理死亡逻辑的细节 */
    void HandleDeath(AActor* Killer);

    // 如果你想使用引擎内置的伤害系统 (通过 Actor 的 TakeDamage 函数触发)，
    // 你可以在 BeginPlay 中绑定到 Owner 的 OnTakeAnyDamage 委托，并需要下面这个处理函数。
    // UFUNCTION()
    // void HandleOwnerTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};