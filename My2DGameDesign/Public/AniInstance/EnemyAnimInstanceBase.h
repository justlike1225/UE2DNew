// My2DGameDesign/Public/AniInstance/EnemyAnimInstanceBase.h (Refactored)
#pragma once

#include "CoreMinimal.h"
#include "PaperZDAnimInstance.h"
#include "Interfaces/AnimationListener//EnemyMovementAnimListener.h" // 包含并实现通用接口
#include "Interfaces/AnimationListener//EnemyStateAnimListener.h"    // 包含并实现通用接口
#include "EnemyAnimInstanceBase.generated.h"

// --- 前向声明 ---
class AEnemyCharacterBase;         // 敌人角色基类
class UCharacterMovementComponent; // 角色移动组件

/**
 * @brief 所有敌人动画蓝图的基础 C++ 类 (重构后)。
 * 只包含通用状态变量，并实现通用的监听器接口 (移动、状态)。
 * 特定能力（如攻击）的状态和接口实现应放在子类中。
 */
UCLASS() // 如果不希望直接在编辑器中创建这个基类的蓝图，可以加上 Abstract
class MY2DGAMEDESIGN_API UEnemyAnimInstanceBase : public UPaperZDAnimInstance,
                                                 public IEnemyMovementAnimListener, // 只实现这两个通用接口
                                                 public IEnemyStateAnimListener
{
    GENERATED_BODY()

public:
    // 构造函数
    UEnemyAnimInstanceBase();

protected:
    // --- 通用状态变量 ---
    // 这些变量由通用接口函数更新，或在 Tick 中更新，并在动画蓝图的状态机转换条件中使用。

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement", meta = (AllowPrivateAccess = "true"))
    float Speed = 0.0f; // 当前速度大小

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsFalling = true; // 是否正在下落 (初始设为 true 较安全)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving = false; // 是否正在移动

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | State", meta = (AllowPrivateAccess = "true"))
    bool bIsHurt = false; // 是否处于受击状态

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | State", meta = (AllowPrivateAccess = "true"))
    bool bIsDead = false; // 是否已死亡

    // --- 【移除】以下特定能力的状态变量 ---
    // bool bIsAttackingMelee = false;
    // bool bIsAttackingRanged = false;
    // bool bIsTeleporting = false;
    // --- 【移除完毕】 ---


    // --- PaperZD 和 UAnimInstance 的生命周期函数 (保留) ---
    virtual void OnInit_Implementation() override;
    virtual void OnTick_Implementation(float DeltaTime) override;


    // --- 通用接口函数的 C++ 实现声明 ---
    // 这些函数将响应来自外部的通知，并更新上面的通用状态变量。

    // IEnemyMovementAnimListener
    virtual void OnMovementStateChanged_Implementation(float InSpeed, bool bInIsFalling, bool bInIsMoving) override;

    // IEnemyStateAnimListener
    virtual void OnDeathState_Implementation(AActor* Killer) override;
    virtual void OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection, bool bInterruptsCurrentAction) override;

    // --- 【移除】以下特定接口函数的 C++ 实现声明 ---
    // virtual void OnMeleeAttackStarted_Implementation(AActor* Target) override;
    // virtual void OnRangedAttackStarted_Implementation(AActor* Target) override;
    // virtual void OnTeleportStateChanged_Implementation(bool bNewIsTeleporting) override;
    // --- 【移除完毕】 ---


    // --- 内部使用的引用 (保留) ---
    UPROPERTY(Transient, BlueprintReadOnly, Category="References", meta=(AllowPrivateAccess="true"))
    TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
};