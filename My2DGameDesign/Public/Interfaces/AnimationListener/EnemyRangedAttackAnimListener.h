// My2DGameDesign/Public/Interfaces/EnemyRangedAttackAnimListener.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyRangedAttackAnimListener.generated.h" // 注意生成的头文件名

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyRangedAttackAnimListener : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 接口，用于动画实例接收与敌人远程攻击相关的通知。
 */
class MY2DGAMEDESIGN_API IEnemyRangedAttackAnimListener
{
	GENERATED_BODY()
public:
	/**
	 * @brief 当敌人开始执行远程攻击动作时调用。
	 * @param Target (可选) 攻击的目标 Actor。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Combat")
	void OnRangedAttackStarted(AActor* Target);

	// 你未来可以根据需要添加更多函数，比如：
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Combat")
	// void OnRangedAttackEnded();
};