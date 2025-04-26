#include "AnimationNotify/StartHeroSweepTraceNotify.h"
#include "PaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/HeroCombatComponent.h"

void UStartHeroSweepTraceNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	Super::OnReceiveNotify_Implementation(OwningInstance); // 父类

	if(OwningInstance) {
		APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());
		if (Hero) {
			UHeroCombatComponent* CombatComp = Hero->GetHeroCombatComponent();
			if (CombatComp) {
				// 调用 CombatComponent 的函数，并传入持续时间
				CombatComp->StartSweepTrace(TraceDuration);
			}
		}
	}
}