

#pragma once

#include "CoreMinimal.h"
#include "Enum/EnemyMeleeAttackType.h"
#include "UObject/Interface.h"
#include "MeleeAbilityExecutor.generated.h"

class AActor;



UINTERFACE(MinimalAPI, Blueprintable)
class UMeleeAbilityExecutor : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IMeleeAbilityExecutor
{
	GENERATED_BODY()

public:
	/**
	 * 请求执行一次指定的近战攻击。
	 * @param AttackType 要执行的攻击类型或索引。
	 * @param Target 可选的攻击目标 Actor。
	 * @return 如果攻击成功启动，返回 true；否则返回 false。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Ability|Melee")
	bool ExecuteMeleeAttack(EEnemyMeleeAttackType AttackType, AActor* Target); 
	
};