#pragma once
#include "CoreMinimal.h"
#include "AnimationNotify/BaseAttackNotify.h"
#include "ThrustAttackNotify.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UThrustAttackNotify : public UBaseAttackNotify
{
	GENERATED_BODY()

public:
	virtual FName GetAttackShapeIdentifier() const override;
};
