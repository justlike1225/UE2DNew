// HeroUpwardSweepSettingsDA.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HeroUpwardSweepSettingsDA.generated.h"

UCLASS(BlueprintType)
class MY2DGAMEDESIGN_API UHeroUpwardSweepSettingsDA : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta=(ClampMin="0.0"))
	float RageCost = 30.0f; // 怒气消耗

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta=(ClampMin="0.0"))
	float Damage = 40.0f; // 伤害值

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta=(ClampMin="0.0"))
	float Cooldown = 3.0f; // 冷却时间 (秒)

	// 可以添加其他参数，如攻击范围、持续时间等
};