// My2DGameDesign/Private/AnimationNotify/Enemies/FinishTeleportNotify.cpp
#include "AnimationNotify/Enemies/FinishTeleportNotify.h"
#include "PaperZDAnimInstance.h"
#include "GameFramework/Actor.h"
#include "Components/TeleportComponent.h" // 包含传送组件头文件

void UFinishTeleportNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance) return;

	AActor* OwnerActor = OwningInstance->GetOwningActor();
	if (!OwnerActor) return;

	UTeleportComponent* TeleportComp = OwnerActor->FindComponentByClass<UTeleportComponent>();
	if (TeleportComp)
	{
		UE_LOG(LogTemp, Log, TEXT("FinishTeleportNotify: Calling FinishTeleportState on TeleportComponent of %s"), *OwnerActor->GetName());
		TeleportComp->FinishTeleportState();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FinishTeleportNotify: Cannot find TeleportComponent on Actor '%s'."), *OwnerActor->GetName());
	}
}