// My2DGameDesign/Private/AnimationNotify/Enemy/EnemyActivateMeleeCollisionNotify.cpp
#include "AnimationNotify/Enemies/EnemyActivateMeleeCollisionNotify.h"
#include "PaperZDAnimInstance.h"          // 需要获取 Owning Actor
#include "GameFramework/Actor.h"          // Actor 基类
#include "Components/EnemyMeleeAttackComponent.h" // 需要调用攻击组件

void UEnemyActivateMeleeCollisionNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyActivateMeleeCollisionNotify: OwningInstance is null."));
		return;
	}

	// 1. 获取拥有此动画实例的 Actor
	AActor* OwnerActor = OwningInstance->GetOwningActor();
	if (!OwnerActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyActivateMeleeCollisionNotify: Cannot get Owning Actor from AnimInstance '%s'."), *OwningInstance->GetName());
		return;
	}

	// 2. 在 Owner Actor 上查找近战攻击组件
	UEnemyMeleeAttackComponent* MeleeAttackComp = OwnerActor->FindComponentByClass<UEnemyMeleeAttackComponent>();
	if (!MeleeAttackComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyActivateMeleeCollisionNotify: Cannot find EnemyMeleeAttackComponent on Actor '%s'."), *OwnerActor->GetName());
		return;
	}

	// 3. 调用组件的激活函数，传入在此通知实例中设置的参数
	MeleeAttackComp->ActivateMeleeCollision(ShapeIdentifier, Duration);
	// 不需要在这里检查返回值，组件内部会处理能否激活的逻辑
}