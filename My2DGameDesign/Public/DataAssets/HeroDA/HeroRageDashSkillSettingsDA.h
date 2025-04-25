#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HeroRageDashSkillSettingsDA.generated.h"

UCLASS(BlueprintType)
class MY2DGAMEDESIGN_API UHeroRageDashSkillSettingsDA : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta=(ClampMin="0.0"))
	float RageCost = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta=(ClampMin="0.0"))
	float DamageAmount = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta=(ClampMin="0.1"))
	float DashSpeed = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta=(ClampMin="0.01"))
	float DashDuration = 0.3f; // 冲刺移动效果的持续时间

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta=(ClampMin="0.0"))
	float Cooldown = 2.0f;
	
};