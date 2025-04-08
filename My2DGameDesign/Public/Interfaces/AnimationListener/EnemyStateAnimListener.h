// My2DGameDesign/Public/Interfaces/EnemyStateAnimListener.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Math/Vector.h" // 需要 FVector
#include "EnemyStateAnimListener.generated.h" // 注意生成的头文件名

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyStateAnimListener : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 接口，用于动画实例接收通用的敌人状态更新（如死亡、受击）。
 */
class MY2DGAMEDESIGN_API IEnemyStateAnimListener
{
	GENERATED_BODY()
public:
	/**
	 * @brief 当敌人死亡时调用。
	 * @param Killer (可选) 杀死敌人的 Actor。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | State")
	void OnDeathState(AActor* Killer);

	/**
	 * @brief 当敌人受到伤害时调用 (在死亡判定之前)。
	 * @param DamageAmount 本次受到的伤害量。
	 * @param HitDirection (可选) 伤害来源的方向向量。
	 * @param bInterruptsCurrentAction 是否应该尝试打断当前动作。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | State")
	void OnTakeHit(float DamageAmount, const FVector& HitDirection, bool bInterruptsCurrentAction);
};