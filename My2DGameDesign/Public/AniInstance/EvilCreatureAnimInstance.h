// My2DGameDesign/Public/AniInstance/EvilCreatureAnimInstance.h
#pragma once

#include "CoreMinimal.h"
#include "AniInstance/EnemyAnimInstanceBase.h"
// 包含需要实现的 Listener 接口
#include "Interfaces/AnimationListener/EnemyMeleeAttackAnimListener.h"
#include "Interfaces/AnimationListener/EnemyTeleportAnimListener.h"
#include "Interfaces/AnimationListener/MeleeStateResetListener.h"
#include "EvilCreatureAnimInstance.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UEvilCreatureAnimInstance : public UEnemyAnimInstanceBase,
                                                     public IEnemyMeleeAttackAnimListener, // 实现近战接口
                                                     public IEnemyTeleportAnimListener,
                                                     public IMeleeStateResetListener // 实现传送接口
{
	GENERATED_BODY()

protected:
	// --- 添加的状态变量 ---

	/** 是否正在执行近战攻击？ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Combat",
		meta = (AllowPrivateAccess = "true"))
	bool bIsAttackingMelee = false;

	/** 当前正在执行的近战攻击索引 (0 代表未攻击, 1 代表攻击1, 2 代表攻击2, etc.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Combat",
		meta = (AllowPrivateAccess = "true"))
	int32 MeleeAttackIndex = 0; // 用于区分两种攻击

	/** 是否正在执行传送？ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Ability",
		meta = (AllowPrivateAccess = "true"))
	bool bIsTeleporting = false;

	// --- 接口函数的 C++ 实现声明 ---

	// IEnemyMeleeAttackAnimListener
	virtual void OnMeleeAttackStarted_Implementation(AActor* Target) override;
	// 注意：我们假设一个 OnMeleeAttackStarted 就够了，具体的攻击动画由 AI/Component 决定播放哪个
	// 如果需要更明确地区分，可以修改接口或组件逻辑，传递攻击索引

	// IEnemyTeleportAnimListener
	virtual void OnTeleportStateChanged_Implementation(bool bNewIsTeleporting) override;
public:
	virtual void HandleMeleeAttackEnd_Implementation() override; // 注意是 _Implementation
};
