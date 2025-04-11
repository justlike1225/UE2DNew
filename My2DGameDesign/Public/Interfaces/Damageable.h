#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage | Combat")
	float ApplyDamage(float DamageAmount, AActor* DamageCauser, AController* InstigatorController,
	                  const FHitResult& HitResult);
};
