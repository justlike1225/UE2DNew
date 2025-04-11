#pragma once
#include "CoreMinimal.h"
#include "AnimationNotify/BaseAttackNotify.h"
#include "OnAttackBoxNotify.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UOnAttackBoxNotify : public UBaseAttackNotify
{
	GENERATED_BODY()

public:
	virtual FName GetAttackShapeIdentifier() const override;
};
