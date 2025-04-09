// My2DGameDesign/Public/AnimationNotify/Enemy/EnemyActivateMeleeCollisionNotify.h
#pragma once

#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h" // 继承 PaperZD 的基类
#include "EnemyActivateMeleeCollisionNotify.generated.h"

// 前向声明
class UEnemyMeleeAttackComponent;

/**
 * @brief C++ 动画通知，用于在动画特定帧激活敌人近战攻击组件的碰撞体。
 */
UCLASS(DisplayName="Enemy: Activate Melee Collision") // 在编辑器中显示的名称
class MY2DGAMEDESIGN_API UEnemyActivateMeleeCollisionNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

public:
	/**
	 * @brief 要激活的碰撞体的标识符名称。
	 * 这个名称需要与 AEvilCreature 中定义的 FName 匹配 (例如 EvilCreatureAttackShapeNames::MeleeNormal)。
	 * 可以在动画编辑器中为每个通知实例单独设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify | Melee Collision")
	FName ShapeIdentifier = FName("MeleeNormal"); // 提供一个默认值

	/**
	 * @brief 碰撞体保持激活状态的持续时间（秒）。
	 * 可以在动画编辑器中为每个通知实例单独设置。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify | Melee Collision", meta = (ClampMin = "0.01"))
	float Duration = 0.15f; // 提供一个默认值

protected:
	/** 当动画播放到此通知时，引擎会调用此函数 */
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};