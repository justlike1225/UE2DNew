// My2DGameDesign/Private/AniInstance/EvilCreatureAnimInstance.cpp
#include "AniInstance/EvilCreatureAnimInstance.h"
#include "GameFramework/Actor.h" // For AActor* Target

// --- 接口实现 ---

void UEvilCreatureAnimInstance::OnMeleeAttackStarted_Implementation(AActor* Target)
{
	// UE_LOG(LogTemp, Verbose, TEXT("EvilCreatureAnimInstance: Received OnMeleeAttackStarted."));
	this->bIsAttackingMelee = true;
	// 如何设置 MeleeAttackIndex？
	// 方案1: 依赖 AI/Component。AI 任务在调用 ExecuteAttack 前，可以在 Blackboard 设置一个 "NextAttackIndex"
	// 然后 AnimInstance 可以尝试从 Owner 的 Blackboard 读取这个值。这稍微有点耦合。
	// 方案2: 修改 MeleeAttackComponent::ExecuteAttack 和 IEnemyMeleeAttackAnimListener::OnMeleeAttackStarted 接口，让它们传递一个 AttackIndex 参数。
	// 方案3: 动画状态机自己管理？比如进入 Attack1 状态就把 Index 设为 1。

	// 我们暂时假设动画状态机或蓝图逻辑会处理 MeleeAttackIndex 的具体设置
	// 这里只标记开始攻击
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
