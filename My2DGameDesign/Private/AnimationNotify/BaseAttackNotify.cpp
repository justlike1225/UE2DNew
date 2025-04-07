

// My2DGameDesign/Private/AnimationNotify/BaseAttackNotify.cpp
#include "AnimationNotify/BaseAttackNotify.h"
#include "PaperZDAnimInstance.h"
#include "PaperZDCharacter_SpriteHero.h"
#include "Components/HeroCombatComponent.h" // 包含战斗组件头文件
#include "TimerManager.h" // 仍然需要 Timer

void UBaseAttackNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const
{
	if (!OwningInstance) return;

	APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwningInstance->GetOwningActor());
	if (!Hero)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseAttackNotify: 无法获取有效角色!"));
		return;
	}

	// 获取战斗组件
	UHeroCombatComponent* CombatComp = Hero->GetHeroCombatComponent();
	if (!CombatComp)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseAttackNotify: 角色 '%s' 上找不到 HeroCombatComponent!"), *Hero->GetName());
		return;
	}

	// 获取要激活的形状标识符
	FName ShapeIdentifier = GetAttackShapeIdentifier();
	if (ShapeIdentifier == NAME_None)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseAttackNotify: GetAttackShapeIdentifier() 返回了 NAME_None!"));
		return;
	}

	// 调用战斗组件的激活函数
	CombatComp->ActivateAttackCollision(ShapeIdentifier, AttackDuration);

}