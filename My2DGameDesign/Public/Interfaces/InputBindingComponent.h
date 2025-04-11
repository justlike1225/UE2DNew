#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InputBindingComponent.generated.h"

class UEnhancedInputComponent;

UINTERFACE(MinimalAPI, Blueprintable)
class UInputBindingComponent : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IInputBindingComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input Binding")
	void BindInputActions(UEnhancedInputComponent* EnhancedInputComponent);
	virtual void BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent) = 0;
};
