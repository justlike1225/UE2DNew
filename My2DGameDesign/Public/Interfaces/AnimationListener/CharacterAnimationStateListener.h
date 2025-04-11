#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Math/Vector.h"
#include "CharacterAnimationStateListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCharacterAnimationStateListener : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API ICharacterAnimationStateListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnIntentStateChanged(bool bNewIsWalking, bool bNewIsRunning);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnDashStateChanged(bool bNewIsDashing);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnCombatStateChanged(int32 NewComboCount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnJumpRequested();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation State Listener")
	void OnAirAttackStateChanged(bool bNewIsAirAttacking);
};
