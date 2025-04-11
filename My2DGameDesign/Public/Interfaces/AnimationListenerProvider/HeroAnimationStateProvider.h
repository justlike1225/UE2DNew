#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ScriptInterface.h"
#include "Interfaces/AnimationListener//CharacterAnimationStateListener.h"
#include "HeroAnimationStateProvider.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UHeroAnimationStateProvider : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IHeroAnimationStateProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider")
	TScriptInterface<ICharacterAnimationStateListener> GetAnimStateListener() const;
};
