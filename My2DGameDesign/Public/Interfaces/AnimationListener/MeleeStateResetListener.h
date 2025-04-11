#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MeleeStateResetListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UMeleeStateResetListener : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IMeleeStateResetListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Combat Reset")
	void HandleMeleeAttackEnd();
};
