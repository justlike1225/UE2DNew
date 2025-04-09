// My2DGameDesign/Public/AnimationNotify/Enemy/EnemyResetMeleeStateNotify.h
#pragma once

#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "EnemyResetMeleeStateNotify.generated.h"

// 前向声明 (如果需要)
class UEvilCreatureAnimInstance; // <-- 需要前向声明我们要 Cast 到的具体动画实例类

/**
 * @brief C++ 动画通知，用于在近战攻击动画结束时，调用动画实例的函数来重置相关状态。
 */
UCLASS(DisplayName="Enemy: Reset Melee State") // 在编辑器中显示的名称
class MY2DGAMEDESIGN_API UEnemyResetMeleeStateNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

protected:
	/** 当动画播放到此通知时，引擎会调用此函数 */
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};