// My2DGameDesign/Public/Interfaces/InputBindingComponent.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InputBindingComponent.generated.h"

class UEnhancedInputComponent; // 前向声明

// UInterface 类 - UE反射系统需要
UINTERFACE(MinimalAPI, Blueprintable) // Blueprintable 可选，如果也想让蓝图组件实现
class UInputBindingComponent : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief C++接口，用于允许Actor组件绑定输入动作。
 * 拥有者Actor（如Character）会在其 SetupPlayerInputComponent 中查找并调用此接口。
 */
class MY2DGAMEDESIGN_API IInputBindingComponent
{
	GENERATED_BODY()

public:
	/**
	 * @brief 组件实现此函数以绑定它们关心的输入动作。
	 * @param EnhancedInputComponent 从拥有者Actor传递过来的增强输入组件。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input Binding") // BlueprintNativeEvent 允许C++和蓝图都实现
	void BindInputActions(UEnhancedInputComponent* EnhancedInputComponent);
	virtual void BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent) = 0; // C++ 纯虚函数，强制实现
};	