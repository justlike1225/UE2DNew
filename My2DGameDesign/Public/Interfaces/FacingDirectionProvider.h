// My2DGameDesign/Public/Interfaces/FacingDirectionProvider.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Math/Vector.h" // 需要 FVector
#include "FacingDirectionProvider.generated.h"

// UInterface boilerplate
UINTERFACE(MinimalAPI, Blueprintable) // Blueprintable 可选，如果蓝图 Actor 也想实现
class UFacingDirectionProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief C++ 接口，用于 Actor 提供其当前的面向或期望移动方向。
 * DashComponent 或其他需要方向的组件可以查询 Owner 是否实现此接口。
 */
class MY2DGAMEDESIGN_API IFacingDirectionProvider
{
	GENERATED_BODY()

public:
	/**
	 * @brief 获取该 Actor 当前的面向方向向量。
	 * @return 返回一个单位向量，表示面向的方向 (例如 FVector(1,0,0) 或 FVector(-1,0,0))。
	 * 实现者负责根据自身逻辑 (如 Sprite Scale, AI 状态等) 提供正确的方向。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Direction Provider")
	FVector GetFacingDirection() const; // 声明为 BlueprintNativeEvent 允许 C++ 和蓝图实现
};