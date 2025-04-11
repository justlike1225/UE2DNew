#include "AniInstance/EvilCreatureAnimInstance.h"
#include "GameFramework/Actor.h"

void UEvilCreatureAnimInstance::OnMeleeAttackStarted_Implementation(AActor* Target, int32 AttackIndex)
{
	this->MeleeAttackIndex = AttackIndex;
	this->bIsAttackingMelee = true;

	// 这里可以添加更多的逻辑，比如播放攻击动画或其他效果
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Melee Attack Started"));
}


void UEvilCreatureAnimInstance::OnTeleportStateChanged_Implementation(bool bNewIsTeleporting)
{
	this->bIsTeleporting = bNewIsTeleporting;


	if (bNewIsTeleporting)
	{
		this->bIsAttackingMelee = false;
		this->MeleeAttackIndex = 0;
	}
}

void UEvilCreatureAnimInstance::HandleMeleeAttackEnd_Implementation()
{
	this->bIsAttackingMelee = false;
	this->MeleeAttackIndex = 0;
}
