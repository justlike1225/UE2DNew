// 文件: Public/States/DeadState.h
#pragma once

#include "CoreMinimal.h"
#include "States/HeroStateBase.h"
#include "DeadState.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UDeadState : public UHeroStateBase
{
	GENERATED_BODY()

public:
	virtual void OnEnterState_Implementation() override;

	
	
};