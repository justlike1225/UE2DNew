// 文件路径: My2DGameDesign/Private/AnimationNotify/RageDashExecuteMovementNotify.cpp
#include "AnimationNotify/RageDashExecuteMovementNotify.h"
#include "PaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/Skills/RageDashComponent.h"

void URageDashExecuteMovementNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance) return;

	
	APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());


	if (Hero)
	{
		if (URageDashComponent* RageDashComp = Hero->GetRageDashComponent())
		{
		
			RageDashComp->ExecuteRageDashMovement();
		}
	

	}

}
