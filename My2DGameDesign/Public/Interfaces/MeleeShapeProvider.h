// My2DGameDesign/Public/Interfaces/MeleeShapeProvider.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MeleeShapeProvider.generated.h"

class UPrimitiveComponent; // 前向声明

UINTERFACE(MinimalAPI, Blueprintable)
class UMeleeShapeProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 接口，用于 Actor 提供用于近战命中检测的 UPrimitiveComponent。
 */
class MY2DGAMEDESIGN_API IMeleeShapeProvider
{
	GENERATED_BODY()

public:
	/**
	 * @brief 获取与给定标识符关联的、用于近战命中检测的 Primitive 组件。
	 * @param ShapeIdentifier 标识所需形状的名称。
	 * @return 对应的 UPrimitiveComponent 指针，如果找不到或不适用则返回 nullptr。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Melee Shape Provider")
	UPrimitiveComponent* GetMeleeShapeComponent(FName ShapeIdentifier) const;
};