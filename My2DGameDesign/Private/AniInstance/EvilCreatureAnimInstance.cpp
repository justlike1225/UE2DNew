// My2DGameDesign/Private/AniInstance/EvilCreatureAnimInstance.cpp
#include "AniInstance/EvilCreatureAnimInstance.h"
#include "GameFramework/Actor.h" // For AActor* Target

// --- 接口实现 ---

void UEvilCreatureAnimInstance::OnMeleeAttackStarted_Implementation(AActor* Target)
{
	// UE_LOG(LogTemp, Verbose, TEXT("EvilCreatureAnimInstance: Received OnMeleeAttackStarted."));
	this->bIsAttackingMelee = true;
	UE_LOG(LogTemp, Log, TEXT("EvilCreatureAnimInstance: OnMeleeAttackStarted called. bIsAttackingMelee set to: %s, MeleeAttackIndex set to: %d"),
		this->bIsAttackingMelee ? TEXT("True") : TEXT("False"),
		this->MeleeAttackIndex);
}
void UEvilCreatureAnimInstance::OnTeleportStateChanged_Implementation(bool bNewIsTeleporting)
{
	// UE_LOG(LogTemp, Verbose, TEXT("EvilCreatureAnimInstance: Received OnTeleportStateChanged - New State: %s"), bNewIsTeleporting ? TEXT("True") : TEXT("False"));
	this->bIsTeleporting = bNewIsTeleporting;

	// 如果传送开始，可能需要重置其他状态，例如停止攻击
	if(bNewIsTeleporting)
	{
		this->bIsAttackingMelee = false;
		this->MeleeAttackIndex = 0;
	}
}

void UEvilCreatureAnimInstance::HandleMeleeAttackEnd_Implementation()
{
	// 把原来的逻辑放在这里
	// UE_LOG(LogTemp, Verbose, TEXT("EvilCreatureAnimInstance: Received HandleMeleeAttackEnd via Interface."));
	this->bIsAttackingMelee = false;
	this->MeleeAttackIndex = 0;
}
