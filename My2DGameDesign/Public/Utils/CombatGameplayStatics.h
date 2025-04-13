// 文件路径: Public/Utils/CombatGameplayStatics.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GenericTeamAgentInterface.h" // 需要包含阵营接口头文件
#include "CombatGameplayStatics.generated.h"

class AActor; // 前向声明 Actor

UCLASS()
class MY2DGAMEDESIGN_API UCombatGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief 检查攻击者是否可以对受害者造成伤害，基于阵营关系。
	 * 通常用于防止友军伤害。
	 * @param Attacker 攻击方 Actor。
	 * @param Victim 受击方 Actor。
	 * @return 如果可以造成伤害 (非友方关系，或无法判断阵营)，返回 true；如果是友方，返回 false。
	 */
	UFUNCTION(BlueprintPure, Category = "Combat|Teams", meta = (DisplayName = "Can Attacker Damage Victim (Team Check)"))
	static bool CanDamageActor(const AActor* Attacker, const AActor* Victim);

private:
	/** 内部辅助函数，用于获取 Actor 的 TeamAgent 接口 (优先检查 Controller) */
	static const IGenericTeamAgentInterface* GetTeamAgentInterfaceFromActor(const AActor* Actor);
};