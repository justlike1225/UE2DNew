#include "AnimationNotify/ThrustAttackNotify.h"
#include "Components/HeroCombatComponent.h"

FName UThrustAttackNotify::GetAttackShapeIdentifier() const
{
	return AttackShapeNames::ThrustAttackCapsule;
}
