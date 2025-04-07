#include "AnimationNotify/SpawnSwordBeamNotify.h" // 头文件
#include "PaperZDAnimInstance.h"                 // 动画实例基类
#include "PaperZDCharacter_SpriteHero.h"         // 角色类
#include "Components/HeroCombatComponent.h"      // 战斗组件

void USpawnSwordBeamNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance) return;

	APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());
	if (!Hero)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnSwordBeamNotify: Cannot get valid Hero character from AnimInstance."));
		return;
	}

	UHeroCombatComponent* CombatComp = Hero->GetHeroCombatComponent();
	if (!CombatComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnSwordBeamNotify: Cannot find HeroCombatComponent on the character."));
		return;
	}

	// 调用战斗组件的生成函数
	CombatComp->HandleAnimNotify_SpawnSwordBeam();
}