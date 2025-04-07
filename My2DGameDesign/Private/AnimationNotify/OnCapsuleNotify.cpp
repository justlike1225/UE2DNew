
// My2DGameDesign/Private/AnimationNotify/OnCapsuleNotify.cpp
#include "AnimationNotify/OnCapsuleNotify.h"
#include "Components/HeroCombatComponent.h" // 包含定义 FName 的头文件

FName UOnCapsuleNotify::GetAttackShapeIdentifier() const
{
	return AttackShapeNames::AttackHitCapsule; // 返回定义的常量名称
}