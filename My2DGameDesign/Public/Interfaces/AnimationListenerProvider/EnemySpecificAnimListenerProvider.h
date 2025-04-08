// My2DGameDesign/Public/Interfaces/EnemySpecificAnimListenerProvider.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ScriptInterface.h" // 需要 TScriptInterface

// 包含所有可能的 Listener 接口头文件
// 这样实现这个 Provider 接口的类（如 EnemyCharacterBase）就不需要再单独包含了
#include "Interfaces/AnimationListener/EnemyMovementAnimListener.h"
#include "Interfaces/AnimationListener/EnemyStateAnimListener.h"
#include "Interfaces/AnimationListener/EnemyMeleeAttackAnimListener.h"
#include "Interfaces/AnimationListener/EnemyRangedAttackAnimListener.h"
#include "Interfaces/AnimationListener/EnemyTeleportAnimListener.h"
// 如果未来还有其他 Listener 接口，也要加在这里

#include "EnemySpecificAnimListenerProvider.generated.h" // 注意生成的头文件名

// --- 前向声明 ---
// 如果接口函数需要返回特定类，可以在这里前向声明
// class UPaperZDAnimInstance; // TScriptInterface 不需要前向声明模板参数

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemySpecificAnimListenerProvider : public UInterface
{
    GENERATED_BODY()
};

/**
 * @brief 接口，允许 Actor (特指敌人) 提供对其各种 *具体* 动画状态监听器接口的访问。
 * 实现者 (如 AEnemyCharacterBase) 应负责找到其动画实例，并尝试将其转换为请求的接口。
 */
class MY2DGAMEDESIGN_API IEnemySpecificAnimListenerProvider
{
    GENERATED_BODY()
public:
    /**
     * @brief 获取实现了移动相关动画监听器接口的对象。
     * @return 返回一个 TScriptInterface。如果动画实例未实现该接口或无效，则返回的接口无效。
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
    TScriptInterface<IEnemyMovementAnimListener> GetMovementAnimListener() const;

    /**
     * @brief 获取实现了通用状态（死亡、受击）动画监听器接口的对象。
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
    TScriptInterface<IEnemyStateAnimListener> GetStateAnimListener() const;

    /**
     * @brief 获取实现了近战攻击动画监听器接口的对象。
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
    TScriptInterface<IEnemyMeleeAttackAnimListener> GetMeleeAttackAnimListener() const;

    /**
     * @brief 获取实现了远程攻击动画监听器接口的对象。
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
    TScriptInterface<IEnemyRangedAttackAnimListener> GetRangedAttackAnimListener() const;

    /**
     * @brief 获取实现了传送能力动画监听器接口的对象。
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
    TScriptInterface<IEnemyTeleportAnimListener> GetTeleportAnimListener() const;

    // --- (可选) 提供基础 AnimInstance 的访问 ---
    // 如果某些系统确实需要直接访问 AnimInstance (虽然应尽量避免)，可以保留这个
    /**
     * @brief (可选) 获取基础的 PaperZD 动画实例对象指针。
     * @warning 尽量通过具体的 Listener 接口交互，避免直接依赖 AnimInstance 类型。
     * @return 返回 UPaperZDAnimInstance 指针，如果无效则为 nullptr。
     */
    // UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
    // class UPaperZDAnimInstance* GetBaseAnimInstance() const;
};