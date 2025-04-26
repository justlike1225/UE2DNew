// My2DGameDesign/Private/AnimationNotify/FinishUpwardSweepNotify.cpp
#include "AnimationNotify/FinishUpwardSweepNotify.h"
#include "PaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h" // 需要 Hero 类

void UFinishUpwardSweepNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance) return;

	APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());
	if (Hero)
	{
		// 调用 Hero 身上用于结束技能状态的函数
		Hero->FinishUpwardSweep();
	
	}

}