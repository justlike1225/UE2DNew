// My2DGameDesign/Public/AniInstance/HeroPaperZDAnimInstance.h
#pragma once

#include "CoreMinimal.h"
#include "PaperZDAnimInstance.h"
#include "Interfaces/CharacterAnimationStateListener.h" // <-- 包含接口头文件
#include "HeroPaperZDAnimInstance.generated.h"

// 前向声明
class UCharacterMovementComponent; // <-- 仍然需要移动组件

// --- 让动画实例实现 ICharacterAnimationStateListener 接口 ---
UCLASS()
class MY2DGAMEDESIGN_API UHeroPaperZDAnimInstance : public UPaperZDAnimInstance, public ICharacterAnimationStateListener
{
	GENERATED_BODY()

protected:
	// --- 缓存的状态变量 (供 ABP 读取) ---
	// 这些变量现在主要由接口函数更新，或者在 Tick 中更新物理状态

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsMovingOnGround = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = true; // 初始假设在空中

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsWalking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsRunning = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement", meta = (AllowPrivateAccess = "true"))
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement", meta = (AllowPrivateAccess = "true"))
	float GroundSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement", meta = (AllowPrivateAccess = "true"))
	float VerticalSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Dash", meta = (AllowPrivateAccess = "true"))
	bool bIsDashing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Combat", meta = (AllowPrivateAccess = "true"))
	int32 ComboCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAirAttacking = false; // 角色是否正在执行空中攻击

	// --- 保留或获取对移动组件的引用 (用于Tick更新物理状态) ---
	UPROPERTY() // 内部使用
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponentPtr;


	// --- 重写 PaperZD 的初始化和更新函数 ---
	virtual void OnInit_Implementation() override;
	virtual void OnTick_Implementation(float DeltaTime) override;

	// --- ICharacterAnimationStateListener 接口函数的 C++ 实现声明 ---
	virtual void OnIntentStateChanged_Implementation(bool bNewIsWalking, bool bNewIsRunning) override;
	virtual void OnDashStateChanged_Implementation(bool bNewIsDashing) override;
	virtual void OnCombatStateChanged_Implementation(int32 NewComboCount) override;
	virtual void OnJumpRequested_Implementation() override;
	virtual void OnAirAttackStateChanged_Implementation(bool bNewIsAirAttacking) override;

};