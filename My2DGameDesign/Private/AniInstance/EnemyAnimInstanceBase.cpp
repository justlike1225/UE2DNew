#include "AniInstance/EnemyAnimInstanceBase.h"
#include "Enemies/EnemyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "My2DGameDesign/My2DGameDesign.h"

UEnemyAnimInstanceBase::UEnemyAnimInstanceBase()
{
	Speed = 0.0f;
	bIsFalling = true;
	bIsMoving = false;
	bIsHurt = false;
	bIsDead = false;
}


void UEnemyAnimInstanceBase::EnemyResetHurtState()
{
	
	if (this->bIsHurt)
	{
		UE_LOG(LogTemp, Log, TEXT("EnemyAnimInstance: EnemyResetHurtState called - Setting bIsHurt to false."));
		this->bIsHurt = false;
	}
	
}


void UEnemyAnimInstanceBase::OnInit_Implementation()
{
	Super::OnInit_Implementation();

	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwningActor());
	if (OwnerEnemyCharacter.IsValid())
	{
		OwnerMovementComponent = OwnerEnemyCharacter->GetCharacterMovement();
		if (!OwnerMovementComponent.IsValid())
		{
		}
	}
}

void UEnemyAnimInstanceBase::OnTick_Implementation(float DeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);

	// 确保我们有有效的移动组件引用 (如果之前获取失败，尝试再次获取)
	if (!OwnerMovementComponent.IsValid())
	{
		if (OwnerEnemyCharacter.IsValid())
		{
			OwnerMovementComponent = OwnerEnemyCharacter->GetCharacterMovement();
		}

		// 如果仍然无法获取移动组件，设置状态为静止/坠落，然后返回
		if (!OwnerMovementComponent.IsValid())
		{
			this->Speed = 0.0f;
			this->bIsMoving = false;
			this->bIsFalling = true; // 假设无法获取时可能在空中或未初始化
			return;
		}
	}

	// --- 从移动组件获取状态 ---
	this->bIsFalling = OwnerMovementComponent->IsFalling(); // 获取是否在空中/坠落
	FVector Velocity = OwnerMovementComponent->Velocity; // 获取当前速度向量


	this->Speed = Velocity.Size2D();

	
	this->bIsMoving = this->Speed > KINDA_SMALL_NUMBER; // KINDA_SMALL_NUMBER 通常是 1e-4f

	
}

void UEnemyAnimInstanceBase::OnMovementStateChanged_Implementation(float InSpeed, bool bInIsFalling, bool bInIsMoving)
{
	this->Speed = InSpeed;
	this->bIsFalling = bInIsFalling;
	this->bIsMoving = bInIsMoving;
}


void UEnemyAnimInstanceBase::OnDeathState_Implementation(AActor* Killer)
{
	this->bIsDead = true;


	this->bIsHurt = false;
	this->bIsMoving = false;
	this->Speed = 0.0f;


	JumpToNode(AnimationJumpNodeName::EnemyDeath);
}


void UEnemyAnimInstanceBase::OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection, bool bInterruptsCurrentAction)
{
	// 检查是否允许中断，以及角色是否已死亡
	if (bInterruptsCurrentAction && !bIsDead)
	{
		// --- 添加判断 ---
		// 只有当角色当前不处于受击状态时，才触发新的受击状态
		if (!this->bIsHurt)
		{
			UE_LOG(LogTemp, Log, TEXT("EnemyAnimInstance: OnTakeHit - First hit detected. Setting bIsHurt=true and jumping to HurtEntry."));
			this->bIsHurt = true; // 标记进入受击状态
			this->bIsMoving = false; // 视觉上停止移动
			JumpToNode(AnimationJumpNodeName::EnemyHurt); // 强制跳转到 Hurt 状态
		}
		
	}
}
