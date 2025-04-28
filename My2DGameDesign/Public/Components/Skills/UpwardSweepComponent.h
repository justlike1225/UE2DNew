#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/InputBindingComponent.h" 
#include "InputActionValue.h" 
#include "TimerManager.h" 
#include "UpwardSweepComponent.generated.h"
class UHeroUpwardSweepSettingsDA;
class UInputAction;
class UEnhancedInputComponent;
class APaperZDCharacter_SpriteHero;
class URageComponent;
class UCharacterMovementComponent;
class ICharacterAnimationStateListener;
class UPrimitiveComponent;
template <class InterfaceType> class TScriptInterface;
struct FHitResult;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpWardSweepCooldownTick, float, RemainingTime);
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MY2DGAMEDESIGN_API UUpwardSweepComponent : public UActorComponent, public IInputBindingComponent
{
    GENERATED_BODY()
public:
    UUpwardSweepComponent();
    UFUNCTION(BlueprintPure, Category = "Skill | Upward Sweep | Status")
    bool IsPerformingUpwardSweep() const { return bIsPerformingUpwardSweep; }
    UFUNCTION(BlueprintPure, Category = "Skill | Upward Sweep | Status")
    bool CanExecuteUpwardSweep() const;
    UFUNCTION(BlueprintCallable, Category = "Skill | Upward Sweep | Actions")
    void TryExecuteUpwardSweep();
    UFUNCTION(BlueprintCallable, Category = "Skill | Upward Sweep | AnimNotify")
    void StartSweepTrace(float Duration);
    UFUNCTION(BlueprintCallable, Category = "Skill | Upward Sweep | AnimNotify")
    void StopSweepTrace();
    UFUNCTION(BlueprintCallable, Category = "Skill | Upward Sweep | AnimNotify")
    void FinishUpwardSweep(); 
    virtual void BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent) override;
    UFUNCTION(BlueprintPure, Category="UpwardSweep | Status")
    float GetCooldownRemaining() const;
    void BroadcastCooldownTick();
    UPROPERTY(BlueprintAssignable, Category="Rage Dash|Events")
    FOnUpWardSweepCooldownTick OnUpswpCooldownTick;
    FTimerHandle CooldownTickTimer;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    UPROPERTY(EditDefaultsOnly, Category = "Skill | Upward Sweep | Configuration")
    TObjectPtr<UHeroUpwardSweepSettingsDA> UpwardSweepSettings;
    UPROPERTY(EditDefaultsOnly, Category = "Skill | Upward Sweep | Configuration | Input")
    TObjectPtr<UInputAction> UpwardSweepAction; 
    /**
     * 定义上扫攻击动画中，攻击判定点的【关键帧】相对偏移位置。
     * 需要与下面的 KeyframeTimes 数组一一对应。
     * 这些偏移量是相对于角色 Sprite 组件局部坐标系的。
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill | Upward Sweep | Keyframes", meta=(ToolTip="Keyframe relative offsets for the Upward Sweep attack point."))
    TArray<FVector> UpwardSweepKeyframeOffsets;
    /**
     * 定义上面每个关键帧偏移量对应的时间点 (归一化时间, 0.0 to 1.0)。
     * 必须与 KeyframeOffsets 数组大小相同且按时间顺序排列 (0.0代表判定开始, 1.0代表判定结束)。
     * 例如: [0.0, 0.3, 0.7, 1.0]
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill | Upward Sweep | Keyframes", meta=(ToolTip="Normalized time points (0-1) corresponding to each keyframe offset. Must match size of KeyframeOffsets and be ordered."))
    TArray<float> UpwardSweepKeyframeTimes;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Skill | Upward Sweep | Status", meta=(AllowPrivateAccess="true"))
    bool bIsPerformingUpwardSweep = false;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Skill | Upward Sweep | Status", meta=(AllowPrivateAccess="true"))
    bool bIsUpwardSweepOnCooldown = false;
    FTimerHandle UpwardSweepCooldownTimer;
    FTimerHandle AttackTraceTimerHandle; 
    UPROPERTY(Transient)
    TWeakObjectPtr<APaperZDCharacter_SpriteHero> OwnerHero;
    UPROPERTY(Transient)
    TWeakObjectPtr<URageComponent> OwnerRageComponent;
    UPROPERTY(Transient)
    TWeakObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
    UPROPERTY(Transient)
    TScriptInterface<ICharacterAnimationStateListener> OwnerAnimListener;
    bool bIsTrackingAttackPoint = false;
    FVector PreviousAttackPointWorldLocation;
    UPROPERTY(Transient)
    TSet<TObjectPtr<AActor>> HitActorsThisSweep;
    float AttackTraceStartTime = 0.0f;
    float AttackTraceDuration = 0.0f;
private:
    void ExecuteUpwardSweep();
    UFUNCTION() 
    void OnUpwardSweepCooldownFinished();
    /** 处理来自 Enhanced Input 的输入触发 */
    UFUNCTION() 
    void HandleUpwardSweepInputTriggered(const FInputActionValue& Value);
    /** 获取动画监听器接口的辅助函数 */
    TScriptInterface<ICharacterAnimationStateListener> GetAnimListener() ;
    /** Sweep Trace Tick 函数 */
    UFUNCTION()
    void PerformSweepTraceTick();
    /** 根据归一化时间获取插值偏移 */
    FVector GetInterpolatedOffset(float NormalizedTime) const;
     /** 获取世界定时器管理器的辅助函数 */
    FTimerManager& GetWorldTimerManager() const;
};