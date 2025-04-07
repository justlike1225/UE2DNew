// My2DGameDesign/Private/AnimationNotify/OnAttackBoxNotify.cpp
#include "AnimationNotify/OnAttackBoxNotify.h"
#include "Components/HeroCombatComponent.h" // 包含定义 FName 的头文件

FName UOnAttackBoxNotify::GetAttackShapeIdentifier() const
{
	return AttackShapeNames::AttackHitBox; // 返回定义的常量名称
}