#include "AnimationNotify/RageDashExecuteMovementNotify.h" // 包含你自己的头文件
#include "PaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h" // 需要包含 Hero 类

void URageDashExecuteMovementNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance) return;

	// 获取拥有此动画实例的 Actor，并尝试转换为 Hero 类
	APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());

	// 如果转换成功，调用我们新增的函数
	if (Hero)
	{
		Hero->ExecuteRageDashMovement();
	}
	
}