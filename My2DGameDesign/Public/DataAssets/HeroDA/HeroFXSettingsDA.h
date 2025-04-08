#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include  "Actors/AfterImageActor.h"
#include "HeroFXSettingsDA.generated.h"

/**
 * @brief 存储英雄视觉效果（如残影）相关配置的数据资产。
 */
UCLASS(BlueprintType)
class MY2DGAMEDESIGN_API UHeroFXSettingsDA : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- 残影设置 (Afterimage) ---
	

	/** @brief 残影效果使用的基础材质。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterimage Effect")
	TObjectPtr<UMaterialInterface> AfterImageBaseMaterial; // 新增
	/** @brief 生成残影的时间间隔（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterimage Effect", meta = (ClampMin = "0.01"))
	float AfterImageInterval = 0.05f;

	/** @brief 单个残影的生存时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterimage Effect", meta = (ClampMin = "0.01"))
	float AfterImageLifetime = 0.3f;

	/** @brief 残影淡出效果使用的材质参数名称（必须是 Scalar Parameter）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterimage Effect")
	FName AfterImageOpacityParamName = FName("Opacity"); // 提供默认值

	/** @brief 残影生成时的初始不透明度。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterimage Effect", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AfterImageInitialOpacity = 0.5f;

	/** @brief 残影淡出效果的更新频率（秒）。 值越小越平滑，但性能开销略高。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Afterimage Effect", meta = (ClampMin = "0.01"))
	float AfterImageFadeUpdateInterval = 0.03f;

	// 可以添加其他特效相关的设置
};