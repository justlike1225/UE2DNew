#include "Enemies/GhostWarriorCharacter.h"

#include "PaperFlipbookComponent.h"
#include "Components/EnemyMeleeAttackComponent.h" // 包含近战组件头文件
#include "Components/CapsuleComponent.h"       // 包含胶囊体头文件
#include "GameFramework/CharacterMovementComponent.h" // 包含移动组件头文件
#include "PaperZDAnimationComponent.h"        // 用于获取动画实例
#include "PaperZDAnimInstance.h"            // 用于获取动画实例
#include "Interfaces/AnimationListener/MeleeStateResetListener.h" // 用于重置近战状态的接口

AGhostWarriorCharacter::AGhostWarriorCharacter()
{
	// 1. 创建必要的组件
	MeleeAttackComponent = CreateDefaultSubobject<UEnemyMeleeAttackComponent>(TEXT("MeleeAttackComponent"));

	// 创建攻击碰撞体
	AttackShape = CreateDefaultSubobject<UCapsuleComponent>(TEXT("AttackShape"));

	
	
}

// 在 PostInitializeComponents 或 BeginPlay 中进行附加和最终确认
void AGhostWarriorCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents(); // 调用父类实现

    // 确保 Sprite 存在后再附加碰撞体
    if (AttackShape && GetSprite() && AttackShape->GetAttachParent() != GetSprite())
    {
        AttackShape->AttachToComponent(GetSprite(), FAttachmentTransformRules::KeepRelativeTransform);
        AttackShape->IgnoreActorWhenMoving(this, true); // 附加后再忽略自身碰撞
    }
	if (AttackShape)
	{
		// 给碰撞体打上标签，方便 GetMeleeShapeComponent 按名字查找
		AttackShape->ComponentTags.Add(GhostWarriorAttackShapeNames::SwordSlash);
		// --- 配置碰撞体属性 (这些只是示例值，需要根据你的美术资源调整) ---
		AttackShape->SetRelativeLocation(FVector(6.88f, 0.0f, 0.54f)); // 假设在角色前方
		AttackShape->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f)); // 胶囊体通常需要旋转90度才横置
		AttackShape->SetCapsuleSize(11.0f, 36.0f); // 半径和半高

		// --- 配置碰撞设置 (与 EvilCreature 保持一致) ---
		AttackShape->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 默认禁用，由动画通知开启
		AttackShape->SetCollisionProfileName(TEXT("OverlapOnlyPawn")); // 或者你定义的专门用于敌人攻击的 Profile
		AttackShape->SetCollisionResponseToAllChannels(ECR_Ignore);
		AttackShape->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 只检测 Pawn
		AttackShape->CanCharacterStepUpOn = ECB_No;
		AttackShape->SetGenerateOverlapEvents(true);
		// 碰撞事件的绑定放到了 MeleeAttackComponent 内部的 ActivateMeleeCollision 中处理，这里不用绑
	}

	// 2. 配置为飞行单位
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// 设置默认移动模式为飞行
		MoveComp->SetMovementMode(MOVE_Flying);
		
		

		// 确保 2D 行为的设置
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->RotationRate = FRotator::ZeroRotator;
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));

		if (UPaperFlipbookComponent* SpriteTmp = GetSprite())
		{
			SpriteTmp->AddRelativeLocation(FVector(4.5f, 0.0f, 1.69f));
		}
		
      
		bAssetFacesRightByDefault = false;
	}
}

void AGhostWarriorCharacter::BeginPlay()
{
    Super::BeginPlay(); // 调用父类实现

    // 再次确认攻击碰撞初始是关闭的
    if (AttackShape)
    {
        AttackShape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}


// --- 接口实现 ---

UPrimitiveComponent* AGhostWarriorCharacter::GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const
{
	// 如果只有一个攻击碰撞体，可以直接判断并返回
	if (AttackShape && AttackShape->ComponentHasTag(ShapeIdentifier))
	{
		return AttackShape;
	}
	UE_LOG(LogTemp, Warning, TEXT("%s: GetMeleeShapeComponent - Could not find shape with identifier: %s"), *GetNameSafe(this), *ShapeIdentifier.ToString());
	return nullptr;
}

bool AGhostWarriorCharacter::CanPerformMeleeAttack_Implementation() const
{
	// 委托给 MeleeAttackComponent 判断
	return MeleeAttackComponent ? MeleeAttackComponent->CanAttack() : false;
}

bool AGhostWarriorCharacter::CanPerformTeleport_Implementation() const
{
	// 这个敌人不会传送
	return false;
}

bool AGhostWarriorCharacter::IsPerformingMeleeAttack_Implementation() const
{
	// 委托给 MeleeAttackComponent 判断
	return MeleeAttackComponent ? MeleeAttackComponent->IsAttacking() : false;
}

bool AGhostWarriorCharacter::IsPerformingTeleport_Implementation() const
{
	// 这个敌人不会传送
	return false;
}

bool AGhostWarriorCharacter::ExecuteMeleeAttack_Implementation(EEnemyMeleeAttackType AttackType, AActor* Target) 
{
	// 委托给 MeleeAttackComponent 执行，传递 AttackType
	if (MeleeAttackComponent)
	{
		return MeleeAttackComponent->ExecuteAttack(AttackType, Target);
		// 或者: return MeleeAttackComponent->ExecuteAttack(AttackIndex, Target);
	}
	UE_LOG(LogTemp, Warning, TEXT("AGhostWarriorCharacter::ExecuteMeleeAttack: MeleeAttackComponent is null!"));
	return false;
}

void AGhostWarriorCharacter::HandleAnim_ActivateMeleeCollision_Implementation(FName ShapeIdentifier, float Duration)
{
	// 委托给 MeleeAttackComponent 处理
	if (MeleeAttackComponent)
	{
		MeleeAttackComponent->ActivateMeleeCollision(ShapeIdentifier, Duration);
	}
    else { UE_LOG(LogTemp, Warning, TEXT("%s: HandleAnim_ActivateMeleeCollision - MeleeAttackComponent is null!"), *GetNameSafe(this)); }
}

void AGhostWarriorCharacter::HandleAnim_FinishTeleportState_Implementation()
{
	// 这个敌人没有传送，此函数可以为空，或者打个日志说明
	// UE_LOG(LogTemp, Verbose, TEXT("%s: HandleAnim_FinishTeleportState called, but no action needed."), *GetNameSafe(this));
}

void AGhostWarriorCharacter::HandleAnim_ResetMeleeState_Implementation()
{
	// 这个事件需要通知动画实例重置攻击状态变量
	UE_LOG(LogTemp, Verbose, TEXT("%s: HandleAnim_ResetMeleeState - Notifying AnimInstance."), *GetNameSafe(this));
	UPaperZDAnimationComponent* AnimComp = GetAnimationComponent();
	if (AnimComp)
	{
		UPaperZDAnimInstance* AnimInst = AnimComp->GetAnimInstance();
		// 检查动画实例是否有效，并且是否实现了重置状态的接口
		if (AnimInst && AnimInst->GetClass()->ImplementsInterface(UMeleeStateResetListener::StaticClass()))
		{
			IMeleeStateResetListener::Execute_HandleMeleeAttackEnd(AnimInst);
		}
        else if(AnimInst) { UE_LOG(LogTemp, Warning, TEXT("%s: AnimInstance does not implement IMeleeStateResetListener!"), *GetNameSafe(this)); }
        else { UE_LOG(LogTemp, Warning, TEXT("%s: Could not get AnimInstance in ResetMeleeState."), *GetNameSafe(this)); }
	}
    else { UE_LOG(LogTemp, Warning, TEXT("%s: Could not get AnimationComponent in ResetMeleeState."), *GetNameSafe(this)); }

}