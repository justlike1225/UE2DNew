// HealthBarWidgetInterface.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HealthBarWidgetInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UHealthBarWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IHealthBarWidgetInterface
{
	GENERATED_BODY()
public:
	// 定义一个蓝图可实现的函数来更新血条
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HealthBar")
	void UpdateHealth(float Current, float Max);
};