// 文件路径: Public/Interfaces/AI/Abilities/MeleeAbilityExecutor.h (示例路径)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MeleeAbilityExecutor.generated.h"

// 前向声明目标 Actor 类
class AActor;

// UInterface 元数据
UINTERFACE(MinimalAPI, Blueprintable)
class UMeleeAbilityExecutor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 定义了执行近战攻击能力的接口。
 * AI 行为树任务可以通过此接口命令 Pawn 执行近战攻击，
 * 而无需了解具体的组件或实现方式。
 */
class MY2DGAMEDESIGN_API IMeleeAbilityExecutor
{
	GENERATED_BODY()

public:
	/**
	 * 请求执行一次近战攻击。
	 * @param Target 可选的攻击目标 Actor。某些攻击可能需要目标，某些可能不需要。
	 * @return 如果攻击成功启动（比如不在冷却中），返回 true；否则返回 false。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Ability|Melee")
	bool ExecuteMeleeAttack(AActor* Target); // 目标 Actor 作为参数
};