// My2DGameDesign/Public/Interfaces/ActionInterruptSource.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ActionInterruptSource.generated.h"

UINTERFACE(MinimalAPI) // 通常不需要 Blueprintable
class UActionInterruptSource : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 接口，允许 Actor 广播一个表示高优先级动作将要发生，可能中断其他动作的信号。
 */
class MY2DGAMEDESIGN_API IActionInterruptSource
{
	GENERATED_BODY()
public:
	/**
	 * @brief 广播动作中断信号。
	 * 实现者通常会调用自己内部的委托（如 OnActionWillInterrupt.Broadcast()）。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Action Interrupt")
	void BroadcastActionInterrupt();
};