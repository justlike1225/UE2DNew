#include "AnimationNotify/SpawnSwordBeamNotify.h"
#include "PaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/HeroCombatComponent.h"

void USpawnSwordBeamNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
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


	CombatComp->HandleAnimNotify_SpawnSwordBeam();
}
