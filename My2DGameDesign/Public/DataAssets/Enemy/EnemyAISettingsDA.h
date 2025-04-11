// Public/DataAssets/Enemy/EnemyAISettingsDA.h (示例)
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Perception/AIPerceptionTypes.h" // 需要包含这个头文件以使用 FAISenseAffiliationFilter
#include "EnemyAISettingsDA.generated.h"

UCLASS(BlueprintType)
class MY2DGAMEDESIGN_API UEnemyAISettingsDA : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- 视觉感知设置 (Sight) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings | Perception | Sight", meta=(ClampMin="0.0"))
	float SightRadius = 300.0f; // 视觉半径

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings | Perception | Sight", meta=(ClampMin="0.0"))
	float LoseSightRadius = 400.0f; // 丢失视觉半径

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings | Perception | Sight", meta=(ClampMin="0.0", ClampMax="180.0"))
	float PeripheralVisionAngleDegrees = 180.0f; // 周边视角 (半角)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings | Perception | Sight", meta=(ClampMin="0.0"))
	float SightMaxAge = 5.0f; // 感知信息最大存活时间

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Settings | Perception | Sight")
	FAISenseAffiliationFilter DetectionByAffiliation; // 检测阵营设置


	// 构造函数中可以设置默认的阵营过滤
	UEnemyAISettingsDA()
	{
		DetectionByAffiliation.bDetectEnemies = true;
		DetectionByAffiliation.bDetectNeutrals = false;
		DetectionByAffiliation.bDetectFriendlies = false;
	}
};