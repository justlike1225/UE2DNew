// 文件: Public/States/FallingState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "FallingState.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UFallingState : public UHeroStateBase
{
	GENERATED_BODY()

public:

	virtual void HandleMoveInput_Implementation(const FInputActionValue& Value) override; // Air control
	
	virtual void HandleLanded_Implementation(const FHitResult& Hit) override; // Transition back to ground

};