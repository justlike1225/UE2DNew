// My2DGameDesign/Private/AnimationNotify/Enemy/EnemyResetMeleeStateNotify.cpp
#include "AnimationNotify/Enemies/EnemyResetMeleeStateNotify.h"
#include "PaperZDAnimInstance.h"
#include "AniInstance/EvilCreatureAnimInstance.h" // <-- 包含具体的动画实例头文件

void UEnemyResetMeleeStateNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyResetMeleeStateNotify: OwningInstance is null."));
		return;
	}

	// 1. 尝试将通用的动画实例转换为我们需要的接口类型
	if (IMeleeStateResetListener* ResetListener = Cast<IMeleeStateResetListener>(OwningInstance))
	{
		// 2. 如果转换成功，调用接口函数
		// 使用 Execute_ 前缀是调用接口 UFUNCTION 的安全方式
		IMeleeStateResetListener::Execute_HandleMeleeAttackEnd(OwningInstance);
	}
	else
	{
		// 这个警告现在表示：这个动画实例没有按预期实现 IMeleeStateResetListener 接口
		UE_LOG(LogTemp, Warning, TEXT("EnemyResetMeleeStateNotify: OwningInstance ('%s') does not implement IMeleeStateResetListener."), *OwningInstance->GetName());
		return;
	}
}