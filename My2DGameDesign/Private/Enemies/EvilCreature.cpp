// 文件路径: Private/Enemies/EvilCreature.cpp

#include "Enemies/EvilCreature.h"
#include "Components/EnemyMeleeAttackComponent.h"
#include "Components/TeleportComponent.h"
#include "Components/CapsuleComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperZDAnimationComponent.h"
#include "PaperZDAnimInstance.h"
#include "Interfaces/AnimationListener/MeleeStateResetListener.h"

// --- (构造函数、BeginPlay、PostInitializeComponents、GetMeleeShapeComponent 等保持不变) ---

AEvilCreature::AEvilCreature()
{
	MeleeAttackComponent = CreateDefaultSubobject<UEnemyMeleeAttackComponent>(TEXT("MeleeAttackComponent"));
	TeleportComponent = CreateDefaultSubobject<UTeleportComponent>(TEXT("TeleportComponent"));

	MeleeHit1 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeHit1"));
	MeleeHit1->ComponentTags.Add(EvilCreatureAttackShapeNames::Melee1); // 在构造函数中添加 Tag 更安全
}

void AEvilCreature::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (MeleeHit1)
	{
		// 确保在 Sprite 创建后再附加
		if (GetSprite()) {
			MeleeHit1->AttachToComponent(GetSprite(), FAttachmentTransformRules::KeepRelativeTransform);
		} else {
             MeleeHit1->SetupAttachment(RootComponent); // 作为后备附加到根
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
		MeleeHit1->IgnoreActorWhenMoving(this, true); // 确保忽略自身
	}
}

UPrimitiveComponent* AEvilCreature::GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const
{
    // 使用 FindComponentByTag 比 GetComponentsByTag 更直接（如果确定只有一个）
    TArray<UActorComponent*> FoundComponents = GetComponentsByTag(UPrimitiveComponent::StaticClass(), ShapeIdentifier);
    if (FoundComponents.Num() > 0)
    {
        return Cast<UPrimitiveComponent>(FoundComponents[0]);
    }
	UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::GetMeleeShapeComponent: Could not find component with tag '%s'"), *ShapeIdentifier.ToString());
	return nullptr;
}

void AEvilCreature::BeginPlay()
{
	Super::BeginPlay();
    // 如果 PostInitialize 时 Sprite 还未准备好，在这里再次尝试附加
	if (MeleeHit1 && GetSprite() && MeleeHit1->GetAttachParent() != GetSprite())
	{
		MeleeHit1->AttachToComponent(GetSprite(), FAttachmentTransformRules::KeepRelativeTransform);
        MeleeHit1->IgnoreActorWhenMoving(this, true); // 再次确认忽略自身
	}
    // 确保初始碰撞关闭
    if(MeleeHit1)
    {
        MeleeHit1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}
bool AEvilCreature::CanPerformMeleeAttack_Implementation() const
{
	return MeleeAttackComponent ? MeleeAttackComponent->CanAttack() : false;
}

bool AEvilCreature::CanPerformTeleport_Implementation() const
{
	return TeleportComponent ? TeleportComponent->CanTeleport() : false;
}
bool AEvilCreature::IsPerformingMeleeAttack_Implementation() const
{
	return MeleeAttackComponent ? MeleeAttackComponent->IsAttacking() : false;
}

bool AEvilCreature::IsPerformingTeleport_Implementation() const
{
	return TeleportComponent ? TeleportComponent->IsTeleporting() : false;
}

bool AEvilCreature::ExecuteMeleeAttack_Implementation(EEnemyMeleeAttackType AttackType, AActor* Target)
{
	if (MeleeAttackComponent)
	{
		return MeleeAttackComponent->ExecuteAttack(AttackType, Target);
	}
	UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::ExecuteMeleeAttack: MeleeAttackComponent is null!"));
	return false;
}
bool AEvilCreature::ExecuteTeleportToLocation_Implementation(const FVector& TargetLocation)
{
	if (TeleportComponent)
	{
		return TeleportComponent->ExecuteTeleport(TargetLocation);
	}
    UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::ExecuteTeleportToLocation: TeleportComponent is null!"));
	return false;
}

/** 处理激活近战碰撞 */
void AEvilCreature::HandleAnim_ActivateMeleeCollision_Implementation(FName ShapeIdentifier, float Duration)
{
	if (MeleeAttackComponent)
	{
		MeleeAttackComponent->ActivateMeleeCollision(ShapeIdentifier, Duration);
	}
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::HandleAnim_ActivateMeleeCollision: MeleeAttackComponent is null! Cannot activate collision."));
    }
}

// --- 修改点 ---
/** 处理完成传送状态 (现在由 TeleportComponent 内部处理状态，此通知不再需要转发) */
void AEvilCreature::HandleAnim_FinishTeleportState_Implementation()
{
	// 不再需要调用 TeleportComponent->FinishTeleportState();
	// 这个函数现在可以留空，或者用于触发纯粹的视觉/音效事件（如果需要的话）。
	// 例如: PlayTeleportEndEffect();
	UE_LOG(LogTemp, Verbose, TEXT("AEvilCreature::HandleAnim_FinishTeleportState: Notify received, but no longer controls teleport state. (Check if still needed for FX/SFX)"));
}
// --- 修改点结束 ---

/** 处理重置近战状态 */
void AEvilCreature::HandleAnim_ResetMeleeState_Implementation()
{
    UE_LOG(LogTemp, Verbose, TEXT("AEvilCreature::HandleAnim_ResetMeleeState: Routing call to AnimInstance's MeleeStateResetListener interface."));
	UPaperZDAnimationComponent* AnimComp = GetAnimationComponent();
	if (AnimComp)
	{
        UPaperZDAnimInstance* AnimInst = AnimComp->GetAnimInstance();
        if (AnimInst)
        {
            if (AnimInst->GetClass()->ImplementsInterface(UMeleeStateResetListener::StaticClass()))
            {
                IMeleeStateResetListener::Execute_HandleMeleeAttackEnd(AnimInst);
            }
            else
            {
                 UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::HandleAnim_ResetMeleeState: AnimInstance does not implement IMeleeStateResetListener!"));
            }
        }
         else
        {
            UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::HandleAnim_ResetMeleeState: Could not get AnimInstance from AnimationComponent."));
        }
	}
     else
    {
        UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::HandleAnim_ResetMeleeState: Could not get AnimationComponent."));
    }
}