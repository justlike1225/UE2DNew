#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyMovementAnimListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyMovementAnimListener : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IEnemyMovementAnimListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Movement")
	void OnMovementStateChanged(float Speed, bool bIsFalling, bool bIsMoving);
};
