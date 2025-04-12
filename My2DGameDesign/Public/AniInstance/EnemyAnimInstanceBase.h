#pragma once
#include "CoreMinimal.h"
#include "PaperZDAnimInstance.h"
#include "Interfaces/AnimationListener//EnemyMovementAnimListener.h"
#include "Interfaces/AnimationListener//EnemyStateAnimListener.h"
#include "EnemyAnimInstanceBase.generated.h"

class AEnemyCharacterBase;
class UCharacterMovementComponent;

UCLASS()
class MY2DGAMEDESIGN_API UEnemyAnimInstanceBase : public UPaperZDAnimInstance,
                                                  public IEnemyMovementAnimListener,
                                                  public IEnemyStateAnimListener
{
	GENERATED_BODY()

public:
	UEnemyAnimInstanceBase();
	/**
		* @brief 由 PaperZD 动画状态节点事件调用，用于重置受击状态标志。
		* 需要在 PaperZD 编辑器中将此函数名绑定到 Hurt 状态节点的 "On Exit Function" 或类似事件上。
		*/
	UFUNCTION(BlueprintCallable, Category = "Animation State Events") // BlueprintCallable 允许蓝图子类调用或重写（如果需要）
	void EnemyResetHurtState();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement",
		meta = (AllowPrivateAccess = "true"))
	float Speed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | State",
		meta = (AllowPrivateAccess = "true"))
	bool bIsHurt = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | State",
		meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

	virtual void OnInit_Implementation() override;
	virtual void OnTick_Implementation(float DeltaTime) override;

	virtual void OnMovementStateChanged_Implementation(float InSpeed, bool bInIsFalling, bool bInIsMoving) override;

	virtual void OnDeathState_Implementation(AActor* Killer) override;
	virtual void OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection,
	                                      bool bInterruptsCurrentAction) override;

	UPROPERTY(Transient, BlueprintReadOnly, Category="References", meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
};
