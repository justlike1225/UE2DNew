// 文件路径: My2DGameDesign/Public/AniInstance/HeroPaperZDAnimInstance.h
#pragma once
#include "CoreMinimal.h"
#include "PaperZDAnimInstance.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "HeroPaperZDAnimInstance.generated.h"

class UCharacterMovementComponent;
class AActor; // <--- 前向声明 AActor

UCLASS()
class MY2DGAMEDESIGN_API UHeroPaperZDAnimInstance
	: public UPaperZDAnimInstance, public ICharacterAnimationStateListener
{
	GENERATED_BODY()

public:
	/**动画状态机：退出受伤状态节点自动调用该事件 */
	UFUNCTION(BlueprintCallable, Category = "Animation State Events")
	void ExitHurtAnimStateEvent();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsMovingOnGround = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsWalking = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsRunning = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	FVector Velocity = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	float GroundSpeed = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	float VerticalSpeed = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Dash",
		meta = (AllowPrivateAccess = "true"))
	bool bIsDashing = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Combat",
		meta = (AllowPrivateAccess = "true"))
	int32 ComboCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Combat",
		meta = (AllowPrivateAccess = "true"))
	bool bIsAirAttacking = false;


	/** 标记角色是否已经死亡 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|State",
		meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;


	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponentPtr;

	virtual void OnInit_Implementation() override;
	virtual void OnTick_Implementation(float DeltaTime) override;
	virtual void OnIntentStateChanged_Implementation(bool bNewIsWalking, bool bNewIsRunning) override;
	virtual void OnDashStateChanged_Implementation(bool bNewIsDashing) override;
	virtual void OnCombatStateChanged_Implementation(int32 NewComboCount) override;
	virtual void OnJumpRequested_Implementation() override;
	virtual void OnAirAttackStateChanged_Implementation(bool bNewIsAirAttacking) override;
	virtual void OnFallingRequested_Implementation() override;

	virtual void OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection,
	                                      bool bInterruptsCurrentAction) override;
	virtual void OnDeathState_Implementation(AActor* Killer) override;
};
