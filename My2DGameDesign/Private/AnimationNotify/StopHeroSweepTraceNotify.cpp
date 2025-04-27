#include "AnimationNotify/StopHeroSweepTraceNotify.h"
#include "PaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/Skills/UpwardSweepComponent.h" 
void UStopHeroSweepTraceNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance) return;
	APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());
	if (!Hero) return;
	UUpwardSweepComponent* SweepComp = Hero->FindComponentByClass<UUpwardSweepComponent>();
	if (SweepComp)
	{
		SweepComp->StopSweepTrace();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UStopHeroSweepTraceNotify: Could not find UUpwardSweepComponent on %s."), *Hero->GetName());
	}
}