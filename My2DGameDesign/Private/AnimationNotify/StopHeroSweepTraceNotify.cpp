#include "AnimationNotify/StopHeroSweepTraceNotify.h"
#include "PaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/HeroCombatComponent.h"

void UStopHeroSweepTraceNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	Super::OnReceiveNotify_Implementation(OwningInstance); // 父类

	if(OwningInstance) {
		APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());
		if (Hero) {
			UHeroCombatComponent* CombatComp = Hero->GetHeroCombatComponent();
			if (CombatComp) {
				// 调用 CombatComponent 的停止函数
				CombatComp->StopSweepTrace();
			}
		}
	}
}