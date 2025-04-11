#include "AnimationNotify/Enemies/EnemyActivateMeleeCollisionNotify.h"
#include "PaperZDAnimInstance.h"
#include "GameFramework/Actor.h"
#include "Components/EnemyMeleeAttackComponent.h"

void UEnemyActivateMeleeCollisionNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance)
	{
		return;
	}


	AActor* OwnerActor = OwningInstance->GetOwningActor();
	if (!OwnerActor)
	{
		return;
	}


	UEnemyMeleeAttackComponent* MeleeAttackComp = OwnerActor->FindComponentByClass<UEnemyMeleeAttackComponent>();
	if (!MeleeAttackComp)
	{
		return;
	}


	MeleeAttackComp->ActivateMeleeCollision(ShapeIdentifier, Duration);
}
