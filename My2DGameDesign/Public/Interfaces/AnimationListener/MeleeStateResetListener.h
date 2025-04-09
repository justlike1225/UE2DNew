// My2DGameDesign/Public/Interfaces/AnimationListener/MeleeStateResetListener.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MeleeStateResetListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UMeleeStateResetListener : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 接口，用于动画实例响应近战攻击动画结束的事件。
 */
class MY2DGAMEDESIGN_API IMeleeStateResetListener
{
	GENERATED_BODY()

public:
	/**
	 * @brief 处理近战攻击动画结束的逻辑。
	 * 实现此函数的动画实例通常会在这里重置相关的状态变量。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Combat Reset")
	void HandleMeleeAttackEnd();
};