#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyTeleportAnimListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyTeleportAnimListener : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IEnemyTeleportAnimListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Listener | Ability")
	void OnTeleportStateChanged(bool bNewIsTeleporting);
};
