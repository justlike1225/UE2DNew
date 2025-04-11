#include "AniInstance/EvilCreatureAnimInstance.h"
#include "GameFramework/Actor.h"

void UEvilCreatureAnimInstance::OnMeleeAttackStarted_Implementation(AActor* Target, int32 AttackIndex)
{
	this->MeleeAttackIndex = AttackIndex;
	this->bIsAttackingMelee = true;

	
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
