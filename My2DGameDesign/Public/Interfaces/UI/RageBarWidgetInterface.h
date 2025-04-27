#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RageBarWidgetInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class URageBarWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IRageBarWidgetInterface
{
	GENERATED_BODY()
public:
	/**
	 * 定义一个函数，用于从外部（例如 HUD 类或角色类）命令实现了此接口的 Widget 更新怒气显示。
	 * BlueprintNativeEvent 允许蓝图和 C++ 同时实现。
	 * @param CurrentRage 当前怒气值。
	 * @param MaxRage 最大怒气值。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RageBar")
	void UpdateRage(float CurrentRage, float MaxRage);
};