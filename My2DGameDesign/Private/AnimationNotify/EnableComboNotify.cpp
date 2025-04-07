#include "AnimationNotify/EnableComboNotify.h" // 包含对应的头文件
#include "Actors/PaperZDCharacter_SpriteHero.h" // 需要获取角色类
#include "Components/HeroCombatComponent.h" // 需要获取战斗组件
#include "PaperZDAnimInstance.h" // 动画实例基类

void UEnableComboNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	// 检查动画实例是否有效
	if (!OwningInstance) return;

	// 尝试获取拥有此动画实例的Actor，并转换为我们的角色类
	APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());
	if (!Hero)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnableComboNotify: 无法从动画实例获取有效的 SpriteHero 角色。"));
		return;
	}

	// 从角色身上获取战斗组件
	UHeroCombatComponent* CombatComp = Hero->GetHeroCombatComponent(); // 使用你之前添加的 Getter
	if (!CombatComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnableComboNotify: 角色身上没有找到 HeroCombatComponent。"));
		return;
	}

	// 调用战斗组件的函数来开启连击窗口
	CombatComp->EnableComboInput();
}