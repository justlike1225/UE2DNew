// My2DGameDesign/Private/AniInstance/EnemyAnimInstanceBase.cpp (Refactored)

#include "AniInstance/EnemyAnimInstanceBase.h"
#include "Enemies/EnemyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h" // For GetOwningActor()

// 构造函数
UEnemyAnimInstanceBase::UEnemyAnimInstanceBase()
{
    // 初始化通用状态变量的默认值
    Speed = 0.0f;
    bIsFalling = true; // 初始假设在空中
    bIsMoving = false;
    bIsHurt = false;
    bIsDead = false;
    // 确保移除了对 bIsAttackingMelee, bIsAttackingRanged, bIsTeleporting 的初始化
}

// OnInit_Implementation: 初始化函数 (基本不变)
void UEnemyAnimInstanceBase::OnInit_Implementation()
{
    Super::OnInit_Implementation();

    OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwningActor());
    if (OwnerEnemyCharacter.IsValid())
    {
        OwnerMovementComponent = OwnerEnemyCharacter->GetCharacterMovement();
        if (!OwnerMovementComponent.IsValid())
        {
             UE_LOG(LogTemp, Warning, TEXT("EnemyAnimInstanceBase '%s': Could not get CharacterMovementComponent from Owner '%s'."),
                *GetNameSafe(this), *GetNameSafe(OwnerEnemyCharacter.Get()));
        }
    }
    else
    {
         UE_LOG(LogTemp, Error, TEXT("EnemyAnimInstanceBase '%s': Owning Actor is not derived from AEnemyCharacterBase! Casting failed."), *GetNameSafe(this));
    }
}

// OnTick_Implementation: 每帧更新函数 (基本不变, 只更新通用状态)
void UEnemyAnimInstanceBase::OnTick_Implementation(float DeltaTime)
{
    Super::OnTick_Implementation(DeltaTime);

    // 只更新高频、必要的物理状态
    if (OwnerMovementComponent.IsValid())
    {
        bIsFalling = OwnerMovementComponent->IsFalling();
        // Speed 和 bIsMoving 主要依赖接口更新，但如果需要实时物理速度，可以在这里补充
         // Speed = OwnerMovementComponent->Velocity.Size2D();
         // bIsMoving = Speed > KINDA_SMALL_NUMBER && !bIsFalling; // 简单判断
    }
    else if(OwnerEnemyCharacter.IsValid())
    {
         OwnerMovementComponent = OwnerEnemyCharacter->GetCharacterMovement();
         if(OwnerMovementComponent.IsValid()) { bIsFalling = OwnerMovementComponent->IsFalling(); }
         else { bIsFalling = true; Speed = 0.f; bIsMoving = false; } // 获取不到则重置
    }
    else // Owner 也无效
    {
         bIsFalling = true; Speed = 0.f; bIsMoving = false;
    }
}


// --- 通用接口函数的具体实现 ---

// IEnemyMovementAnimListener
void UEnemyAnimInstanceBase::OnMovementStateChanged_Implementation(float InSpeed, bool bInIsFalling, bool bInIsMoving)
{
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyAnimInstance '%s': OnMovementStateChanged - Speed: %.1f, Falling: %s, Moving: %s"), *GetNameSafe(this), InSpeed, bInIsFalling ? TEXT("True") : TEXT("False"), bInIsMoving ? TEXT("True") : TEXT("False"));
    this->Speed = InSpeed;
    this->bIsFalling = bInIsFalling; // 接受接口传递的下落状态
    this->bIsMoving = bInIsMoving;
}

// IEnemyStateAnimListener
void UEnemyAnimInstanceBase::OnDeathState_Implementation(AActor* Killer)
{
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyAnimInstance '%s': OnDeathState - Killed by: %s"), *GetNameSafe(this), Killer ? *GetNameSafe(Killer) : TEXT("None"));
    this->bIsDead = true; // 设置死亡状态

    // 死亡时重置其他可能冲突的状态
    this->bIsHurt = false;
    this->bIsMoving = false;
    this->Speed = 0.0f;

    // 可能触发跳转到死亡动画状态
     JumpToNode(FName("DeathEntry"));
}

// IEnemyStateAnimListener
void UEnemyAnimInstanceBase::OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection, bool bInterruptsCurrentAction)
{
    // 简单示例：如果允许打断，并且当前不在死亡状态，则进入受击状态
    if (bInterruptsCurrentAction && !bIsDead)
    {
        // UE_LOG(LogTemp, Verbose, TEXT("EnemyAnimInstance '%s': OnTakeHit received. Setting bIsHurt to true."), *GetNameSafe(this));
        this->bIsHurt = true;

        
         this->bIsMoving = false;

        // 触发动画状态机跳转到受击状态
         JumpToNode(FName("HurtEntry"));
        // 注意：你需要在受击动画结束时通过 AnimNotify 或状态机逻辑将 bIsHurt 设回 false
    }
}

 