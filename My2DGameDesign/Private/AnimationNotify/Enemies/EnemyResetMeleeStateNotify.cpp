
#include "AnimationNotify/Enemies/EnemyResetMeleeStateNotify.h" // 包含自身头文件
#include "PaperZDAnimInstance.h"                   // PaperZD 动画实例基类
#include "GameFramework/Actor.h"                   // 需要获取 Actor

#include "Interfaces/AnimationEvents/EnemyAnimationEventHandler.h"

// 重写的接收通知函数
void UEnemyResetMeleeStateNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	// 1. 检查动画实例是否有效
	if (!OwningInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyResetMeleeStateNotify: OwningInstance is null."));
		return;
	}

	// 2. 获取拥有此动画实例的 Actor
	AActor* OwnerActor = OwningInstance->GetOwningActor();
	if (!OwnerActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyResetMeleeStateNotify: OwnerActor is null."));
		return;
	}

	// 3. 检查 OwningActor 是否实现了我们的事件处理器接口
	if (OwnerActor->GetClass()->ImplementsInterface(UEnemyAnimationEventHandler::StaticClass()))
	{
		// 4. 如果实现了接口，就调用接口的 HandleAnim_ResetMeleeState 函数
		IEnemyAnimationEventHandler::Execute_HandleAnim_ResetMeleeState(OwnerActor);
		UE_LOG(LogTemp, Verbose, TEXT("EnemyResetMeleeStateNotify: Called HandleAnim_ResetMeleeState on %s"), *OwnerActor->GetName());
	}
	else
	{
		// 如果 Actor 没有实现接口，发出警告
		UE_LOG(LogTemp, Warning, TEXT("EnemyResetMeleeStateNotify: Owner Actor %s does not implement IEnemyAnimationEventHandler! Notify ignored."), *OwnerActor->GetName());
	}

	
}