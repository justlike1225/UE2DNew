// 文件路径: Private/Enemies/EvilCreature.cpp

#include "Enemies/EvilCreature.h"
#include "Components/EnemyMeleeAttackComponent.h" // <--- 需要近战组件头文件
#include "Components/TeleportComponent.h"      // <--- 需要传送组件头文件
#include "Components/CapsuleComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperZDAnimationComponent.h"      // <--- 需要动画组件头文件 (用于 ResetMeleeState)
#include "PaperZDAnimInstance.h"          // <--- 需要动画实例基类 (用于 ResetMeleeState)
#include "Interfaces/AnimationListener/MeleeStateResetListener.h" // <--- 需要重置监听器接口 (用于 ResetMeleeState)

// --- (已有的构造函数、BeginPlay、GetMeleeShapeComponent_Implementation 等实现保持不变) ---
AEvilCreature::AEvilCreature()
{
	MeleeAttackComponent = CreateDefaultSubobject<UEnemyMeleeAttackComponent>(TEXT("MeleeAttackComponent"));
	TeleportComponent = CreateDefaultSubobject<UTeleportComponent>(TEXT("TeleportComponent"));

	MeleeHit1 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeHit1"));
	
}

void AEvilCreature::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (MeleeHit1)
	{
		// 确保在 Sprite 创建后再附加 (如果构造函数中 GetSprite() 可能为 null)
		// 实际附加逻辑移到 BeginPlay 或 PostInitializeComponents 更安全
		MeleeHit1->SetupAttachment(RootComponent); // 先附加到 Root，BeginPlay 再尝试附加到 Sprite

		MeleeHit1->SetRelativeLocation(FVector(-14.0f, 0.0f, 19.0f));
		MeleeHit1->SetCapsuleHalfHeight(30.0f);
		MeleeHit1->SetCapsuleRadius(8.0f);
		MeleeHit1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
		MeleeHit1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeleeHit1->SetCollisionProfileName(TEXT("OverlapOnlyPawn")); // 或你自定义的 Profile
		MeleeHit1->SetCollisionResponseToAllChannels(ECR_Ignore);
		MeleeHit1->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		MeleeHit1->CanCharacterStepUpOn = ECB_No;
		MeleeHit1->SetGenerateOverlapEvents(true);
		// 添加组件标签，以便 GetMeleeShapeComponent_Implementation 可以通过名称查找
		MeleeHit1->ComponentTags.Add(EvilCreatureAttackShapeNames::Melee1);
	}
}

UPrimitiveComponent* AEvilCreature::GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const
{
	
    TArray<UActorComponent*> Components = GetComponentsByTag(UPrimitiveComponent::StaticClass(), ShapeIdentifier);
    if (Components.Num() > 0)
    {
        return Cast<UPrimitiveComponent>(Components[0]);
    }
   
	return nullptr;
}

void AEvilCreature::BeginPlay()
{
	Super::BeginPlay();
    // 尝试将碰撞体附加到 Sprite (如果 Sprite 有效)
	if (MeleeHit1 && GetSprite() && MeleeHit1->GetAttachParent() != GetSprite())
	{
		MeleeHit1->AttachToComponent(GetSprite(), FAttachmentTransformRules::KeepRelativeTransform);
	}
    // 确保忽略自身碰撞
	if (MeleeHit1)
	{
		MeleeHit1->IgnoreActorWhenMoving(this, true);
        // 通常在 BeginPlay 时再次确认碰撞是关闭的，直到 AnimNotify 激活
        MeleeHit1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
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

bool AEvilCreature::IsPerformingTeleport_Implementation() const
{
	if (TeleportComponent)
	{
		return TeleportComponent->IsTeleporting();
	}
	return false;
}

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


bool AEvilCreature::ExecuteTeleportToLocation_Implementation(const FVector& TargetLocation)
{
	if (TeleportComponent)
	{
		// 调用传送组件的执行函数
		return TeleportComponent->ExecuteTeleport(TargetLocation);
	}
	return false;
}



/** 处理激活近战碰撞 */
void AEvilCreature::HandleAnim_ActivateMeleeCollision_Implementation(FName ShapeIdentifier, float Duration)
{
	// 检查近战攻击组件是否存在
	if (MeleeAttackComponent)
	{
		// 将动画事件的调用转发给近战攻击组件
		UE_LOG(LogTemp, Verbose, TEXT("AEvilCreature::HandleAnim_ActivateMeleeCollision: Routing call to MeleeAttackComponent for Shape '%s' Duration %.2f"), *ShapeIdentifier.ToString(), Duration);
		MeleeAttackComponent->ActivateMeleeCollision(ShapeIdentifier, Duration);
	}
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::HandleAnim_ActivateMeleeCollision: MeleeAttackComponent is null! Cannot activate collision."));
    }
}

/** 处理完成传送状态 */
void AEvilCreature::HandleAnim_FinishTeleportState_Implementation()
{
	// 检查传送组件是否存在
	if (TeleportComponent)
	{
		// 将动画事件的调用转发给传送组件
		UE_LOG(LogTemp, Verbose, TEXT("AEvilCreature::HandleAnim_FinishTeleportState: Routing call to TeleportComponent."));
		TeleportComponent->FinishTeleportState();
	}
     else
    {
        UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::HandleAnim_FinishTeleportState: TeleportComponent is null! Cannot finish state."));
    }
}

/** 处理重置近战状态 */
void AEvilCreature::HandleAnim_ResetMeleeState_Implementation()
{
    // 这个事件原本是通知 AnimInstance 的，所以我们在这里获取 AnimInstance 并调用其接口
	UE_LOG(LogTemp, Verbose, TEXT("AEvilCreature::HandleAnim_ResetMeleeState: Routing call to AnimInstance's MeleeStateResetListener interface."));

    // 尝试获取动画组件和动画实例
	UPaperZDAnimationComponent* AnimComp = GetAnimationComponent(); // UPaperZDCharacter 提供的函数
	if (AnimComp)
	{
        UPaperZDAnimInstance* AnimInst = AnimComp->GetAnimInstance();
        if (AnimInst)
        {
            // 检查 AnimInstance 是否实现了 IMeleeStateResetListener 接口
            if (AnimInst->GetClass()->ImplementsInterface(UMeleeStateResetListener::StaticClass()))
            {
                // 调用 AnimInstance 上的接口函数
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
