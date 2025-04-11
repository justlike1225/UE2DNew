#include "AnimationNotify/EnableComboNotify.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/HeroCombatComponent.h"
#include "PaperZDAnimInstance.h"

void UEnableComboNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
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


	CombatComp->EnableComboInput();
}
