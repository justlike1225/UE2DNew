// My2DGameDesign/Public/AniInstance/EnemyAnimInstanceBase.h

#pragma once

#include "CoreMinimal.h"
#include "PaperZDAnimInstance.h" // 继承自 PaperZD 动画实例基类
#include "Interfaces/EnemyAnimationStateListener.h" // 包含我们定义的敌人动画监听器接口头文件
#include "EnemyAnimInstanceBase.generated.h"       // 生成的头文件

// --- 前向声明 ---
class UCharacterMovementComponent; // 角色移动组件
class AEnemyCharacterBase;         // 敌人角色基类

/**
 * UCLASS 标记此类可被UE识别。
 * 它是所有敌人动画蓝图的基础 C++ 类。
 */
UCLASS()
class MY2DGAMEDESIGN_API UEnemyAnimInstanceBase : public UPaperZDAnimInstance, public IEnemyAnimationStateListener // 公开继承 PaperZD 基类和我们的接口
{
	GENERATED_BODY() // UE类宏

public:
	// 构造函数
	UEnemyAnimInstanceBase();

protected:
	// protected: 只能被本类和子类访问。

	// --- 动画蓝图使用的状态变量 ---
	// 这些变量将由接口函数更新，并在动画蓝图的状态机转换条件中使用。
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly): 让这些变量在动画蓝图编辑器中可见且只读。

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement")
	float Speed = 0.0f; // 当前速度大小

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement")
	bool bIsFalling = false; // 是否正在下落

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement")
	bool bIsMoving = false; // 是否正在移动 (由AI或速度判断)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Combat")
	bool bIsAttackingMelee = false; // 是否正在执行近战攻击动画

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Combat")
	bool bIsAttackingRanged = false; // 是否正在执行远程攻击动画

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | State")
	bool bIsDead = false; // 是否已死亡 (用于触发死亡动画)


	// --- PaperZD 和 UAnimInstance 的生命周期函数 ---
	// OnInit: 类似于 BeginPlay，在动画实例初始化时调用。
	virtual void OnInit_Implementation() override;
	// OnTick: 类似于 Tick，每帧调用。我们尽量减少这里的逻辑。
	virtual void OnTick_Implementation(float DeltaTime) override;

	// --- IEnemyAnimationStateListener 接口函数的 C++ 实现声明 ---
	// 这些函数将响应来自 AI 或组件的通知，并更新上面的状态变量。
	// 使用 _Implementation 后缀是 BlueprintNativeEvent 的要求。

	virtual void OnMovementStateChanged_Implementation(float InSpeed, bool bInIsFalling, bool bInIsMoving) override;
	virtual void OnMeleeAttackStarted_Implementation(AActor* Target) override;
    virtual void OnRangedAttackStarted_Implementation(AActor* Target) override;
    virtual void OnDeathState_Implementation(AActor* Killer) override;
    // 如果你在接口中添加了其他函数，也需要在这里声明对应的 _Implementation 函数。

	// --- 内部使用的引用 ---
	// 使用 TWeakObjectPtr 避免潜在的循环引用问题（虽然 AnimInstance -> Owner 通常还好，但这是好习惯）

	/** 指向拥有此动画实例的敌人角色的弱指针 */
	UPROPERTY(Transient, BlueprintReadOnly, Category="References", meta=(AllowPrivateAccess="true")) // Transient: 不保存, BlueprintReadOnly: 蓝图可读
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;

	/** 指向拥有者角色的移动组件的弱指针 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
};