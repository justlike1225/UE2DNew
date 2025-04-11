#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ActionInterruptSource.generated.h"

UINTERFACE(MinimalAPI)
class UActionInterruptSource : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IActionInterruptSource
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Action Interrupt")
	void BroadcastActionInterrupt();
};
