#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Math/Vector.h"
#include "EnemyStateAnimListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyStateAnimListener : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IEnemyStateAnimListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | State")
	void OnDeathState(AActor* Killer);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | State")
	void OnTakeHit(float DamageAmount, const FVector& HitDirection, bool bInterruptsCurrentAction);
};
