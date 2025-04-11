#include "AniInstance/EnemyAnimInstanceBase.h"
#include "Enemies/EnemyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"

UEnemyAnimInstanceBase::UEnemyAnimInstanceBase()
{
	Speed = 0.0f;
	bIsFalling = true;
	bIsMoving = false;
	bIsHurt = false;
	bIsDead = false;
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

	// 确保我们有有效的移动组件引用
	if (!OwnerMovementComponent.IsValid())
	{
		if (OwnerEnemyCharacter.IsValid()) { OwnerMovementComponent = OwnerEnemyCharacter->GetCharacterMovement(); }

		if (!OwnerMovementComponent.IsValid())
		{
			// 如果无法获取移动组件，设置状态为静止/坠落，然后返回
			this->Speed = 0.0f;
			this->bIsMoving = false;
			return;
		}
	}

	// --- 从移动组件获取状态 ---
	// 检查是否在空中/坠落
	this->bIsFalling = OwnerMovementComponent->IsFalling();

	// 获取当前速度向量
	FVector Velocity = OwnerMovementComponent->Velocity;

	// --- 更新动画实例的变量 ---
	// 计算水平速率 (XY平面上的速度大小)
	this->Speed = FVector::DotProduct(Velocity, OwnerMovementComponent->GetOwner()->GetActorForwardVector());
	// this->Speed = Velocity.Size2D();
	// 判断是否在移动 (速率大于一个很小的阈值)
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


	JumpToNode(FName("DeathEntry"));
}


void UEnemyAnimInstanceBase::OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection,
                                                      bool bInterruptsCurrentAction)
{
	if (bInterruptsCurrentAction && !bIsDead)
	{
		this->bIsHurt = true;


		this->bIsMoving = false;


		JumpToNode(FName("HurtEntry"));
	}
}
