
// --- 修改派生类 ---

// My2DGameDesign/Public/AnimationNotify/OnAttackBoxNotify.h
#pragma once
#include "CoreMinimal.h"
#include "AnimationNotify/BaseAttackNotify.h"
#include "OnAttackBoxNotify.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UOnAttackBoxNotify : public UBaseAttackNotify
{
	GENERATED_BODY()
public:
	// 实现基类函数，返回 Box 的标识符
	virtual FName GetAttackShapeIdentifier() const override;
};

