// My2DGameDesign/Public/Components/HeroCombatComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "Interfaces/InputBindingComponent.h"
#include "HeroCombatComponent.generated.h"

// 前向声明
class ASwordBeamProjectile;
class UHeroCombatSettingsDA;
class APaperZDCharacter; // 改为更通用的基类，如果可能
class UPaperFlipbookComponent; // 需要 Flipbook 来附加
class UPrimitiveComponent;
class UInputAction;
struct FHitResult;
struct FInputActionValue;
class UEnhancedInputComponent;
class ICharacterAnimationStateListener;
template <class InterfaceType>
class TScriptInterface;
class UBoxComponent; // <-- 添加 BoxComponent 前向声明
class UCapsuleComponent; // <-- 添加 CapsuleComponent 前向声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGroundComboStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGroundComboEndedSignature);
// --- 用于 AnimNotify 识别攻击形状的常量名称 ---
namespace AttackShapeNames
{
	const FName AttackHitBox(TEXT("AttackHitBox"));
	const FName AttackHitCapsule(TEXT("AttackHitCapsule"));
	const FName ThrustAttackCapsule(TEXT("ThrustAttackCapsule"));

}

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MY2DGAMEDESIGN_API UHeroCombatComponent : public UActorComponent, public IInputBindingComponent
{
	GENERATED_BODY()

public:
	UHeroCombatComponent();
	/** 当地面连击序列开始时广播 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnGroundComboStartedSignature OnGroundComboStarted;

	/** 当地面连击序列结束（完成、中断或重置）时广播 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnGroundComboEndedSignature OnGroundComboEnded;
	// --- 输入动作 ---
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ComboAttackAction;

	// --- 接口函数 ---
	virtual void BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent) override;

	// --- Getters ---
	UFUNCTION(BlueprintPure, Category = "Combat State")
	int32 GetComboCount() const { return ComboCount; }

	UFUNCTION(BlueprintPure, Category = "Combat State")
	bool CanCombo() const { return bCanCombo; }

	UFUNCTION(BlueprintPure, Category = "Combat State")
	bool IsPerformingAirAttack() const { return bIsPerformingAirAttack; }

	UFUNCTION(BlueprintPure, Category = "Combat State")
	bool CanAirAttack() const { return bCanAirAttack && !bIsPerformingAirAttack; }

	// --- AnimNotify 调用的函数 ---
	UFUNCTION(BlueprintCallable, Category = "Combat|AnimNotify")
	void EnableComboInput();
	UFUNCTION(BlueprintCallable, Category = "Combat|AnimNotify")
	void CloseComboWindowAndSetupResetTimer();
	UFUNCTION(BlueprintCallable, Category = "Combat|AnimNotify")
	void HandleAnimNotify_SpawnSwordBeam();
	UFUNCTION(BlueprintCallable, Category = "Combat|AnimNotify")
	void HandleAnimNotify_AirAttackEnd();
	/** @brief 由 AnimNotify 调用，根据标识符激活指定的攻击碰撞体 */
	UFUNCTION(BlueprintCallable, Category = "Combat|AnimNotify")
	void ActivateAttackCollision(FName ShapeIdentifier, float Duration);

	
	UFUNCTION(BlueprintCallable, Category = "Combat|State")
	void NotifyLanded();
	UFUNCTION()
	void HandleActionInterrupt();

protected:
	
	// --- 组件 ---
	/** @brief 用于普通攻击的盒子碰撞体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> AttackHitBox;

	/** @brief 用于某些攻击的胶囊碰撞体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> AttackHitCapsule;

	/** @brief 用于突刺攻击的胶囊碰撞体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> ThrustAttackCapsule;

	// --- 配置 ---
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UHeroCombatSettingsDA> CombatSettings;

	// --- 内部状态 (保持不变) ---
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat State", meta=(AllowPrivateAccess="true"))
	int32 ComboCount = 0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat State", meta=(AllowPrivateAccess="true"))
	bool bCanCombo = true;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat State", meta=(AllowPrivateAccess="true"))
	bool bIsPerformingAirAttack = false;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat State", meta=(AllowPrivateAccess="true"))
	bool bCanAirAttack = true;

	// --- 计时器 (保持不变, 但可能需要为碰撞体添加新的) ---
	FTimerHandle ResetComboTimer;
	FTimerHandle AttackCooldownTimer;
	FTimerHandle AirAttackCooldownTimer;
	FTimerHandle AttackCollisionTimer; // 用于关闭激活的碰撞体

	// --- 引用 ---
	UPROPERTY() // 缓存拥有者角色
	TWeakObjectPtr<APaperZDCharacter> OwnerCharacter; // 使用基类

	UPROPERTY() // 缓存拥有者角色的Sprite组件，用于附加
	TWeakObjectPtr<UPaperFlipbookComponent> OwnerSpriteComponent;

	// --- 生命周期 ---
	/** 组件初始化，在这里创建和设置碰撞体 */
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override; // 仍然可以用来做一些BeginPlay特定的事
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- 输入处理 (保持不变) ---
	UFUNCTION()
	void HandleAttackInputTriggered(const FInputActionValue& Value);

	// --- 核心逻辑 (保持不变) ---
	void PerformGroundCombo();
	void PerformAirAttack();
	void SpawnSwordBeam();

	// --- 状态重置与冷却 (保持不变) ---
	UFUNCTION()
	void ResetComboState();
	UFUNCTION()
	void OnAttackCooldownFinished();
	UFUNCTION()
	void OnAirAttackCooldownFinished();

	// --- 碰撞处理 ---
	UFUNCTION() // 绑定到所有攻击碰撞体的 OnComponentBeginOverlap
	void OnAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                 int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


private:
	// --- 辅助函数 ---
	/** 获取动画状态监听器接口 */
	TScriptInterface<ICharacterAnimationStateListener> GetAnimListener() const;
	/** 启动地面攻击冷却 */
	void StartAttackCooldown();
	/** 配置攻击碰撞体的通用属性 */
	void ConfigureAttackCollisionComponent(UPrimitiveComponent* CollisionComp,
	                                       FName ProfileName = TEXT("OverlapAllDynamic")); // 可以接受配置名称
	/** 在指定时间后关闭当前激活的攻击碰撞体 */
	UFUNCTION()
	void DeactivateCurrentAttackCollision();

	// --- 缓存的配置值 (保持不变) ---
	float CurrentComboResetDelay = 0.05f;
	float CurrentGroundAttackCooldownDuration = 0.8f;
	float CurrentGroundBaseAttackDamage = 20.0f;
	int32 CurrentMaxGroundComboCount = 3;
	float CurrentAirAttackMeleeDamage = 15.0f;
	float CurrentAirAttackCooldownDuration = 0.6f;
	TSubclassOf<ASwordBeamProjectile> CurrentSwordBeamClass = nullptr;
	FVector CurrentSwordBeamSpawnOffset = FVector(50.0f, 0.0f, 0.0f);
	float CurrentSwordBeamInitialSpeed = 1200.0f;
	float CurrentSwordBeamDamage = 10.0f;
	float CurrentSwordBeamLifeSpan = 2.0f;

	// 记录当前由 AnimNotify 激活的碰撞体，以便 Deactivate
	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> ActiveAttackCollisionShape;
};
