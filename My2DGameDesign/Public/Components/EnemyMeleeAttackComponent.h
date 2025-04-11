// My2DGameDesign/Public/Components/EnemyMeleeAttackComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "Containers/Set.h"
#include "EnemyMeleeAttackComponent.generated.h"

// --- 前向声明 ---
class UEnemyMeleeAttackSettingsDA;
class AEnemyCharacterBase;
class AActor;
class IEnemyMeleeAttackAnimListener;
template<class InterfaceType> class TScriptInterface;
class UPrimitiveComponent; // <--- 添加: 需要 PrimitiveComponent 类型
struct FHitResult;       // <--- 添加: 需要 HitResult 类型
class UDamageable;       // <--- 添加: 需要 Damageable 接口
class AController;       // <--- 添加: 需要 Controller 类型

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MY2DGAMEDESIGN_API UEnemyMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyMeleeAttackComponent();

	// ... (ExecuteAttack, CanAttack, IsAttacking 函数保持不变) ...
	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee")
	bool ExecuteAttack(AActor* Target = nullptr);

	UFUNCTION(BlueprintPure, Category = "Enemy Attack | Melee | Status")
	bool CanAttack() const { return bCanAttack; }

    UFUNCTION(BlueprintPure, Category = "Enemy Attack | Melee | Status")
    bool IsAttacking() const { return bIsAttacking; }

    // ... (ActivateMeleeCollision, DeactivateMeleeCollision 函数保持不变) ...
    UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee | AnimNotify")
    void ActivateMeleeCollision(FName ShapeIdentifier, float Duration = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee | AnimNotify")
    void DeactivateMeleeCollision(FName ShapeIdentifier);


    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Configuration")
	TObjectPtr<UEnemyMeleeAttackSettingsDA> AttackSettings;

    UPROPERTY(Transient)
    TSet<TObjectPtr<AActor>> HitActorsThisSwing;


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ... (内部状态变量 bCanAttack, bIsAttacking 保持不变) ...
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Status", meta=(AllowPrivateAccess="true"))
	bool bCanAttack = true;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Status", meta=(AllowPrivateAccess="true"))
    bool bIsAttacking = false;

	// ... (定时器句柄保持不变) ...
	FTimerHandle AttackCooldownTimer;
    FTimerHandle ActiveCollisionTimerHandle;
    FName ActiveCollisionShapeName;


	UPROPERTY(Transient)
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;

	// --- 新增: 碰撞处理函数 ---
	/** 当激活的近战碰撞体发生重叠时调用 */
	UFUNCTION() // <--- 添加 UFUNCTION 宏
	virtual void HandleAttackOverlap( // <--- 新增函数声明
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	void StartAttackCooldown();

	UFUNCTION()
	void OnAttackCooldownFinished();

    TScriptInterface<IEnemyMeleeAttackAnimListener> GetAnimListener() const;

    void BeginAttackSwing();
};