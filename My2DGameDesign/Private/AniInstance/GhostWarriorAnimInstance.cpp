#include "AniInstance/GhostWarriorAnimInstance.h"
#include "GameFramework/Actor.h" // 可能需要 Actor 类

// 当近战攻击组件通过 AGhostWarriorCharacter -> IEnemySpecificAnimListenerProvider 通知攻击开始时调用
void UGhostWarriorAnimInstance::OnMeleeAttackStarted_Implementation(AActor* Target, int32 AttackIndex)
{
	// 更新状态变量，动画蓝图会检测到这些变化并切换到攻击状态
	this->bIsAttackingMelee = true;
	this->MeleeAttackIndex = AttackIndex; // 记录是哪个攻击动作

	
	UE_LOG(LogTemp, Log, TEXT("GhostWarriorAnimInstance: Melee attack started (Index: %d)"), AttackIndex);
}

// 当攻击动画播放完毕，并通过 EnemyResetMeleeStateNotify -> AGhostWarriorCharacter -> IMeleeStateResetListener 通知时调用
void UGhostWarriorAnimInstance::HandleMeleeAttackEnd_Implementation()
{
	// 重置状态变量，让动画蓝图可以从攻击状态切换回 Idle 或 Fly 状态
	this->bIsAttackingMelee = false;
	this->MeleeAttackIndex = 0; // 重置索引

	UE_LOG(LogTemp, Log, TEXT("GhostWarriorAnimInstance: Melee attack state reset."));
}