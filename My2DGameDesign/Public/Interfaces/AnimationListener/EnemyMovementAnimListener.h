// My2DGameDesign/Public/Interfaces/EnemyMovementAnimListener.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyMovementAnimListener.generated.h" // 注意生成的头文件名

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyMovementAnimListener : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 接口，用于动画实例接收与敌人移动相关的状态更新。
 */
class MY2DGAMEDESIGN_API IEnemyMovementAnimListener
{
	GENERATED_BODY()
public:
	/**
	 * @brief 当敌人的移动相关状态发生变化时调用。
	 * @param Speed 敌人当前的移动速度大小。
	 * @param bIsFalling 敌人当前是否处于下落状态。
	 * @param bIsMoving 敌人当前是否有移动意图或正在移动。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Movement")
	void OnMovementStateChanged(float Speed, bool bIsFalling, bool bIsMoving);
};