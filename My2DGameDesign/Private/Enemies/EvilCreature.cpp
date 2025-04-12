#include "Enemies/EvilCreature.h"
#include "Components/EnemyMeleeAttackComponent.h"
#include "Components/TeleportComponent.h"
#include "Components/CapsuleComponent.h"
#include "PaperFlipbookComponent.h"

AEvilCreature::AEvilCreature()
{
	MeleeAttackComponent = CreateDefaultSubobject<UEnemyMeleeAttackComponent>(TEXT("MeleeAttackComponent"));
	TeleportComponent = CreateDefaultSubobject<UTeleportComponent>(TEXT("TeleportComponent"));

	MeleeHit1 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeHit1"));
	if (MeleeHit1)
	{
		if (GetSprite())
		{
			MeleeHit1->SetupAttachment(GetSprite());
		}
		else
		{
			MeleeHit1->SetupAttachment(RootComponent);
		}

		MeleeHit1->SetRelativeLocation(FVector(-14.0f, 0.0f, 19.0f));
		MeleeHit1->SetCapsuleHalfHeight(30.0f);
		MeleeHit1->SetCapsuleRadius(8.0f);
		MeleeHit1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
		MeleeHit1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeleeHit1->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
		MeleeHit1->SetCollisionResponseToAllChannels(ECR_Ignore);
		MeleeHit1->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		MeleeHit1->CanCharacterStepUpOn = ECB_No;
		MeleeHit1->SetGenerateOverlapEvents(true);
	}
}

UPrimitiveComponent* AEvilCreature::GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const
{
	if (ShapeIdentifier == EvilCreatureAttackShapeNames::Melee1)
	{
		return MeleeHit1;
	}
	return nullptr;
}


void AEvilCreature::BeginPlay()
{
	Super::BeginPlay();
	if (MeleeHit1 && GetSprite() && MeleeHit1->GetAttachParent() != GetSprite())
	{
		MeleeHit1->AttachToComponent(GetSprite(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (MeleeHit1)
	{
		MeleeHit1->IgnoreActorWhenMoving(this, true);
	}
}
/**
 * @brief 实现 ICombatStatusProvider 接口: 检查是否能执行近战攻击
 * @return 如果近战组件有效且可以攻击，返回 true
 */
bool AEvilCreature::CanPerformMeleeAttack_Implementation() const
{
	// 检查 MeleeAttackComponent 是否有效，并调用其 CanAttack() 方法
	if (MeleeAttackComponent)
	{
		return MeleeAttackComponent->CanAttack();
	}
	// 如果没有近战组件，则认为不能攻击
	return false;
}

/**
 * @brief 实现 ICombatStatusProvider 接口: 检查是否能执行传送
 * @return 如果传送组件有效且可以传送，返回 true
 */
bool AEvilCreature::CanPerformTeleport_Implementation() const
{
	// 检查 TeleportComponent 是否有效，并调用其 CanTeleport() 方法
	if (TeleportComponent)
	{
		return TeleportComponent->CanTeleport();
	}
	// 如果没有传送组件，则认为不能传送
	return false;
}
bool AEvilCreature::IsPerformingMeleeAttack_Implementation() const
{
	// 检查 MeleeAttackComponent 是否有效，并调用其 IsPerformingAttack() 方法
	if (MeleeAttackComponent)
	{
		return MeleeAttackComponent->IsAttacking();
	}
	// 如果没有近战组件，则认为没有执行攻击
	return false;
}
/**
 * @brief 实现 IMeleeAbilityExecutor 接口: 执行近战攻击
 * @param Target 攻击的目标 Actor
 * @return 如果 MeleeAttackComponent 成功启动攻击，返回 true
 */
bool AEvilCreature::ExecuteMeleeAttack_Implementation(AActor* Target)
{
	// 检查 MeleeAttackComponent 是否有效，并调用其 ExecuteAttack() 方法
	if (MeleeAttackComponent)
	{
		return MeleeAttackComponent->ExecuteAttack(Target);
	}
	// 如果没有近战组件，则认为无法执行攻击
	return false;
}

/**
 * @brief 实现 ICombatStatusProvider 接口: 检查是否正在传送
 * @return 如果传送组件有效且正在传送，返回 true
 */
bool AEvilCreature::IsPerformingTeleport_Implementation() const
{
	if (TeleportComponent)
	{
		return TeleportComponent->IsTeleporting();
	}
	return false;
}
// --- End ICombatStatusProvider (IsPerformingTeleport) Implementation ---



/**
 * @brief 实现 ITeleportAbilityExecutor 接口: 执行传送到指定位置
 * @param TargetLocation 传送的目标位置
 * @return 如果 TeleportComponent 成功启动传送，返回 true
 */
bool AEvilCreature::ExecuteTeleportToLocation_Implementation(const FVector& TargetLocation)
{
	if (TeleportComponent)
	{
		// 调用传送组件的执行函数
		return TeleportComponent->ExecuteTeleport(TargetLocation);
	}
	return false;
}

