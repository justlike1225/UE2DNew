
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Math/Vector.h" // 需要 FVector
#include "TeleportAbilityExecutor.generated.h"

// UInterface 元数据
UINTERFACE(MinimalAPI, Blueprintable)
class UTeleportAbilityExecutor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 定义了执行传送能力的接口。
 * AI 行为树任务可以通过此接口命令 Pawn 执行传送，
 * 而无需了解具体的组件或实现方式。
 */
class MY2DGAMEDESIGN_API ITeleportAbilityExecutor
{
	GENERATED_BODY()

public:
	/**
	 * 请求执行一次传送到指定位置。
	 * @param TargetLocation 传送的目标世界坐标。
	 * @return 如果传送成功启动（比如不在冷却中），返回 true；否则返回 false。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Ability|Teleport")
	bool ExecuteTeleportToLocation(const FVector& TargetLocation);
};