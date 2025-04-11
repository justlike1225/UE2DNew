#include "AnimationNotify/OnAttackBoxNotify.h"
#include "Components/HeroCombatComponent.h"

FName UOnAttackBoxNotify::GetAttackShapeIdentifier() const
{
	return AttackShapeNames::AttackHitBox;
}
