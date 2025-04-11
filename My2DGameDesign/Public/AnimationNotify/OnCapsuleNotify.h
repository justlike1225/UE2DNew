// My2DGameDesign/Public/AnimationNotify/OnCapsuleNotify.h
#pragma once
#include "CoreMinimal.h"
#include "AnimationNotify/BaseAttackNotify.h"
#include "OnCapsuleNotify.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UOnCapsuleNotify : public UBaseAttackNotify
{
	GENERATED_BODY()

public:
	virtual FName GetAttackShapeIdentifier() const override;
};
