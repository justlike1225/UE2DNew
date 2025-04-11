#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ScriptInterface.h"
#include "Interfaces/AnimationListener/EnemyMovementAnimListener.h"
#include "Interfaces/AnimationListener/EnemyStateAnimListener.h"
#include "Interfaces/AnimationListener/EnemyMeleeAttackAnimListener.h"
#include "Interfaces/AnimationListener/EnemyRangedAttackAnimListener.h"
#include "Interfaces/AnimationListener/EnemyTeleportAnimListener.h"
#include "EnemySpecificAnimListenerProvider.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemySpecificAnimListenerProvider : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IEnemySpecificAnimListenerProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
	TScriptInterface<IEnemyMovementAnimListener> GetMovementAnimListener() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
	TScriptInterface<IEnemyStateAnimListener> GetStateAnimListener() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
	TScriptInterface<IEnemyMeleeAttackAnimListener> GetMeleeAttackAnimListener() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
	TScriptInterface<IEnemyRangedAttackAnimListener> GetRangedAttackAnimListener() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
	TScriptInterface<IEnemyTeleportAnimListener> GetTeleportAnimListener() const;
};
