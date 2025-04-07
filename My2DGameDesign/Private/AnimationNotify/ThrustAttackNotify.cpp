// My2DGameDesign/Private/AnimationNotify/ThrustAttackNotify.cpp
#include "AnimationNotify/ThrustAttackNotify.h"
#include "Components/HeroCombatComponent.h" // 包含定义 FName 的头文件

FName UThrustAttackNotify::GetAttackShapeIdentifier() const
{
	return AttackShapeNames::ThrustAttackCapsule; // 返回定义的常量名称
}