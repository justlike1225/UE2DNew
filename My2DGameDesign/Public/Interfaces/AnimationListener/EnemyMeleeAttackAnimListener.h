#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyMeleeAttackAnimListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyMeleeAttackAnimListener : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IEnemyMeleeAttackAnimListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Combat")
	void OnMeleeAttackStarted(AActor* Target, int32 AttackIndex);
};
