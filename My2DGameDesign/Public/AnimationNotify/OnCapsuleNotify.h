
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
	// 实现基类函数，返回 Capsule 的标识符
	virtual FName GetAttackShapeIdentifier() const override;
};

