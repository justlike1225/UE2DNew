#include "AnimationNotify/CloseComboWindowNotify.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/HeroCombatComponent.h"
#include "PaperZDAnimInstance.h"

void UCloseComboWindowNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
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


	CombatComp->CloseComboWindowAndSetupResetTimer();
}
