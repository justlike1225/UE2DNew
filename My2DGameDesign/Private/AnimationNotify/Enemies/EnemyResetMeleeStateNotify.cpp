#include "AnimationNotify/Enemies/EnemyResetMeleeStateNotify.h"
#include "PaperZDAnimInstance.h"
#include "AniInstance/EvilCreatureAnimInstance.h"

void UEnemyResetMeleeStateNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance)
	{
		return;
	}

	if (IMeleeStateResetListener* ResetListener = Cast<IMeleeStateResetListener>(OwningInstance))
	{
		IMeleeStateResetListener::Execute_HandleMeleeAttackEnd(OwningInstance);
	}
}
