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
	FVector Velocity = OwnerMovementComponent->Velocity;     // 获取当前速度向量

	// --- 更新动画实例的变量 (核心修改处) ---
	// 计算水平面上的速率 (速度向量在XY平面上的大小)
	// Velocity.Size2D() 计算的是 sqrt(Velocity.X^2 + Velocity.Y^2)，总是非负数
	this->Speed = Velocity.Size2D();

	// 判断是否在移动 (速率大于一个很小的阈值)
	// 现在无论向左还是向右移动，只要速度不为零，bIsMoving 就应该是 true
	this->bIsMoving = this->Speed > KINDA_SMALL_NUMBER; // KINDA_SMALL_NUMBER 通常是 1e-4f

	// 可选：如果你还需要区分左右移动的速度（比如用于 BlendSpace），可以单独计算
	// float ForwardSpeedComponent = FVector::DotProduct(Velocity, OwnerMovementComponent->GetOwner()->GetActorForwardVector());
	// 但是动画状态转换通常只关心是否在移动 (bIsMoving) 以及移动速率 (Speed)
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
