#include "AnimationNotify/Enemies/FinishTeleportNotify.h"
#include "PaperZDAnimInstance.h"
#include "GameFramework/Actor.h"
#include "Components/TeleportComponent.h"

void UFinishTeleportNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
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
	UTeleportComponent* TeleportComp = OwnerActor->FindComponentByClass<UTeleportComponent>();
	if (TeleportComp)
	{
		TeleportComp->FinishTeleportState();
	}
}
