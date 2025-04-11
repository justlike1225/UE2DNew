#include "AnimationNotify/BaseAttackNotify.h"
#include "PaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/HeroCombatComponent.h"
#include "TimerManager.h"

void UBaseAttackNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance)
	{
		return;
	}

	APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());
	if (!Hero)
	{
		return;
	}


	UHeroCombatComponent* CombatComp = Hero->GetHeroCombatComponent();
	if (!CombatComp)
	{
		return;
	}


	FName ShapeIdentifier = GetAttackShapeIdentifier();
	if (ShapeIdentifier == NAME_None)
	{
		return;
	}

	CombatComp->ActivateAttackCollision(ShapeIdentifier, AttackDuration);
}
