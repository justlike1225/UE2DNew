
// My2DGameDesign/Public/AnimationNotify/ThrustAttackNotify.h
#pragma once
#include "CoreMinimal.h"
#include "AnimationNotify/BaseAttackNotify.h"
#include "ThrustAttackNotify.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UThrustAttackNotify : public UBaseAttackNotify
{
	GENERATED_BODY()
public:
	// 实现基类函数，返回 Thrust Capsule 的标识符
	virtual FName GetAttackShapeIdentifier() const override;
};