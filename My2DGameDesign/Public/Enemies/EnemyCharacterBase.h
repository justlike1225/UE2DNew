// My2DGameDesign/Public/Enemies/EnemyCharacterBase.h (Refactored)
#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/AnimationListenerProvider//EnemySpecificAnimListenerProvider.h" // <-- 包含新的 Provider 接口
#include "Interfaces/FacingDirectionProvider.h"
#include "EnemyCharacterBase.generated.h"

// --- 前向声明 ---
class UHealthComponent;
class UPaperZDAnimInstance; // <-- 前向声明基础动画实例类
class AAIController;
class UBehaviorTree;
class UEnemyAnimInstanceBase; // 可以保留，如果 GetEnemyAnimInstance 函数仍需要


/**
 * 敌人的基础角色类 (重构后)。
 * 实现新的 Provider 接口，提供获取具体 Listener 接口的方法。
 * 自身不强制要求攻击能力。
 */
UCLASS(Abstract)
class MY2DGAMEDESIGN_API AEnemyCharacterBase : public APaperZDCharacter,
                                           public IDamageable,
                                           public IEnemySpecificAnimListenerProvider, // <-- 实现新的 Provider 接口
                                           public IFacingDirectionProvider
                                           // --- 移除旧的 IEnemyAnimationStateProvider ---
{
    GENERATED_BODY()

public:
    // 构造函数
    AEnemyCharacterBase();

    /** 获取生命组件 (保留) */
    UFUNCTION(BlueprintPure, Category = "Components | Health")
    UHealthComponent* GetHealthComponent() const { return HealthComponent; }

    /** 获取特定敌人动画实例基类 (如果需要，可以保留) */
    UFUNCTION(BlueprintPure, Category="Animation")
    UEnemyAnimInstanceBase* GetEnemyAnimInstance() const; // 注意：这个可能需要调整，取决于你是否还使用EnemyAnimInstanceBase

    // --- 接口实现函数声明 ---

    // --- IDamageable 接口实现 (保留) ---
    virtual float ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser, AController* InstigatorController, const FHitResult& HitResult) override;

    // --- IEnemySpecificAnimListenerProvider 接口实现声明 ---
    virtual TScriptInterface<IEnemyMovementAnimListener> GetMovementAnimListener_Implementation() const override;
    virtual TScriptInterface<IEnemyStateAnimListener> GetStateAnimListener_Implementation() const override;
    virtual TScriptInterface<IEnemyMeleeAttackAnimListener> GetMeleeAttackAnimListener_Implementation() const override;
    virtual TScriptInterface<IEnemyRangedAttackAnimListener> GetRangedAttackAnimListener_Implementation() const override;
    virtual TScriptInterface<IEnemyTeleportAnimListener> GetTeleportAnimListener_Implementation() const override;
    // virtual class UPaperZDAnimInstance* GetBaseAnimInstance_Implementation() const override; // 如果需要实现 GetBaseAnimInstance

    // --- IFacingDirectionProvider 接口实现 (保留) ---
    virtual FVector GetFacingDirection_Implementation() const override;


    /** 指定这个敌人使用的行为树资产 (保留) */
    UPROPERTY(EditDefaultsOnly, Category="AI | Configuration")
    TObjectPtr<UBehaviorTree> BehaviorTree;

protected:
    // 生命周期函数 (保留)
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;

    // 核心组件指针 (保留)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UHealthComponent> HealthComponent;

    // --- 内部状态与引用 ---

    /** 用于缓存基础动画实例指针 (修改：缓存基础类型) */
    UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess = "true"))
    TWeakObjectPtr<UPaperZDAnimInstance> CachedAnimInstancePtr; // <-- 修改为缓存基础指针

    // --- 移除旧的 AnimationStateListener 成员变量 ---
    // TScriptInterface<IEnemyAnimationStateListener> AnimationStateListener; // <-- 移除


    /** 处理死亡 (保留) */
    UFUNCTION()
    virtual void HandleDeath(AActor* Killer);

    /** 设置朝向 (保留) */
    virtual void SetFacingDirection(bool bFaceRight);

    /** 存储当前朝向 (保留) */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="State | Direction", meta=(AllowPrivateAccess="true"))
    bool bIsFacingRight = true;

private:
    /** 辅助函数，尝试查找并缓存基础动画实例指针 (重命名) */
    void CacheBaseAnimInstance(); // <-- 重命名
};