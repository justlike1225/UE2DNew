#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyRangedAttackAnimListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyRangedAttackAnimListener : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IEnemyRangedAttackAnimListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Combat")
	void OnRangedAttackStarted(AActor* Target);
};
