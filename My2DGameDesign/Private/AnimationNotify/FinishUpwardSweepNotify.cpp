#include "AnimationNotify/FinishUpwardSweepNotify.h"
#include "PaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/Skills/UpwardSweepComponent.h" 
void UFinishUpwardSweepNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance) return;
	APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());
	if (!Hero) return;
	UUpwardSweepComponent* SweepComp = Hero->FindComponentByClass<UUpwardSweepComponent>();
	if (SweepComp)
	{
		SweepComp->FinishUpwardSweep();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UFinishUpwardSweepNotify: Could not find UUpwardSweepComponent on %s."), *Hero->GetName());
	}
}