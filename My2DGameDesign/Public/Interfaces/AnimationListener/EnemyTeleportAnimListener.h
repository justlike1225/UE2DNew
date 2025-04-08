// My2DGameDesign/Public/Interfaces/EnemyTeleportAnimListener.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyTeleportAnimListener.generated.h" // 注意生成的头文件名

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyTeleportAnimListener : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 接口，用于动画实例接收与敌人传送能力相关的通知。
 */
class MY2DGAMEDESIGN_API IEnemyTeleportAnimListener
{
	GENERATED_BODY()
public:
	/**
	 * @brief 当敌人的传送状态发生变化时调用。
	 * @param bNewIsTeleporting 敌人当前是否正在传送。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Ability")
	void OnTeleportStateChanged(bool bNewIsTeleporting);
};