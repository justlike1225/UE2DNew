#pragma once

#include "CoreMinimal.h"
#include "Actors/PaperZDCharacter_SpriteHero.h" // 仍然需要角色基类来获取组件
#include "Notifies/PaperZDAnimNotify.h"
#include "BaseAttackNotify.generated.h"

// 前向声明
class UHeroCombatComponent;

UCLASS(Abstract) // 保持 Abstract
class MY2DGAMEDESIGN_API UBaseAttackNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

public:
	// 攻击持续时间
	UPROPERTY(EditAnywhere, Category = "Attack Settings", meta=(ClampMin="0.01"))
	float AttackDuration = 0.2f;

	// PURE_VIRTUAL: 派生类必须实现此函数，返回要激活的攻击形状的标识符FName
	virtual FName GetAttackShapeIdentifier() const PURE_VIRTUAL(UBaseAttackNotify::GetAttackShapeIdentifier, return NAME_None;);

	// 通用实现: 获取战斗组件并调用其激活函数
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};