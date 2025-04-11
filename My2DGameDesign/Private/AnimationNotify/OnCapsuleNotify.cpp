#include "AnimationNotify/OnCapsuleNotify.h"
#include "Components/HeroCombatComponent.h"

FName UOnCapsuleNotify::GetAttackShapeIdentifier() const
{
	return AttackShapeNames::AttackHitCapsule;
}
