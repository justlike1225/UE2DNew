// 文件路径: My2DGameDesign/Public/Components/RageDashComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/InputBindingComponent.h" // 包含输入绑定接口
#include "InputActionValue.h" // 包含 FInputActionValue
#include "RageDashComponent.generated.h"

// 前向声明
class UHeroRageDashSkillSettingsDA;
class UInputAction;
class UEnhancedInputComponent;
class APaperZDCharacter_SpriteHero; // 需要 Owner 类型
class UCharacterMovementComponent;
class URageComponent;
class UCapsuleComponent;
class ICharacterAnimationStateListener;
template <class InterfaceType> class TScriptInterface; // 用于动画监听器

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MY2DGAMEDESIGN_API URageDashComponent : public UActorComponent, public IInputBindingComponent
{
    GENERATED_BODY()

public:
    URageDashComponent();

    // --- 状态查询 ---
    UFUNCTION(BlueprintPure, Category = "Rage Dash | Status")
    bool IsRageDashing() const { return bIsRageDashing; }

    UFUNCTION(BlueprintPure, Category = "Rage Dash | Status")
    bool CanRageDash() const; // 实现将包含冷却和状态检查

    // --- 核心逻辑 ---
    UFUNCTION(BlueprintCallable, Category = "Rage Dash | Actions")
    void TryExecuteRageDash();

    UFUNCTION(BlueprintCallable, Category = "Rage Dash | Actions | Internal")
    void ExecuteRageDashMovement(); // 由动画通知调用

    /** 由 Hero 的碰撞处理调用 */
    UFUNCTION(BlueprintCallable, Category = "Rage Dash | Collision")
    void HandleRageDashHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& SweepResult);

    /** 由外部（如 Hero 的 ActionInterrupt）调用以取消冲刺 */
    UFUNCTION(BlueprintCallable, Category = "Rage Dash | Actions")
    void CancelRageDash();

    // --- IInputBindingComponent 接口实现 ---
    virtual void BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // --- 配置 ---
    UPROPERTY(EditDefaultsOnly, Category = "Rage Dash | Configuration")
    TObjectPtr<UHeroRageDashSkillSettingsDA> RageDashSkillSettings;

    UPROPERTY(EditDefaultsOnly, Category = "Rage Dash | Configuration | Input")
    TObjectPtr<UInputAction> RageDashAction; // 输入动作

    // --- 内部状态 ---
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Rage Dash | Status", meta=(AllowPrivateAccess="true"))
    bool bIsRageDashing = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Rage Dash | Status", meta=(AllowPrivateAccess="true"))
    bool bIsRageDashOnCooldown = false;

    // --- 计时器 ---
    FTimerHandle RageDashMovementTimer;
    FTimerHandle RageDashCooldownTimer;

    // --- 缓存或运行时变量 ---
    float OriginalMovementSpeed = 0.f;
    float OriginalGravity = 1.f;
    TSet<TWeakObjectPtr<AActor>> HitActorsThisDash; // 存储本次冲刺击中的目标

    // --- 内部引用 (在 BeginPlay 获取) ---
    UPROPERTY(Transient) // Transient 表示不保存
    TWeakObjectPtr<APaperZDCharacter_SpriteHero> OwnerHero;

    UPROPERTY(Transient)
    TWeakObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;

    UPROPERTY(Transient)
    TWeakObjectPtr<URageComponent> OwnerRageComponent;

     UPROPERTY(Transient)
    TWeakObjectPtr<UCapsuleComponent> OwnerCapsuleComponent; // 如果碰撞处理需要自己获取

   


private:
  
   
    TScriptInterface<ICharacterAnimationStateListener> OwnerAnimListenerCached;
    // --- 内部函数 ---
    void ExecuteRageDash();
    void EndRageDashMovement();
    UFUNCTION() // 定时器回调需要 UFUNCTION()
    void OnRageDashCooldownFinished();

    /** 处理来自 Enhanced Input 的输入触发 */
    UFUNCTION() // 输入绑定回调也需要 UFUNCTION()
    void HandleRageDashInputTriggered(const FInputActionValue& Value);

    /** 获取动画监听器接口的辅助函数 */
    TScriptInterface<ICharacterAnimationStateListener> GetAnimListener() const;
    FTimerManager& GetWorldTimerManager() const;
};