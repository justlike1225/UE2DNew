// My2DGameDesign/Private/AniInstance/EnemyAnimInstanceBase.cpp

#include "AniInstance/EnemyAnimInstanceBase.h" // 引入对应的头文件
#include "Enemies/EnemyCharacterBase.h"        // 需要获取 Owning Actor 并转换为敌人基类
#include "GameFramework/CharacterMovementComponent.h" // 需要获取移动组件

// 构造函数
UEnemyAnimInstanceBase::UEnemyAnimInstanceBase()
{
    // --- 初始化动画蓝图使用的状态变量的默认值 ---
    Speed = 0.0f;
    bIsFalling = true; // 通常 Actor 生成时可能在空中，设为 true 比较安全
    bIsMoving = false;
    bIsAttackingMelee = false;
    bIsAttackingRanged = false;
    bIsDead = false;
}

// OnInit_Implementation: 初始化函数
void UEnemyAnimInstanceBase::OnInit_Implementation()
{
	Super::OnInit_Implementation(); // 调用父类的初始化逻辑

	// --- 获取对 Owner Actor 和其组件的引用 ---
    // 尝试获取拥有此动画实例的 Actor，并将其转换为 AEnemyCharacterBase 类型
	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwningActor());
	if (OwnerEnemyCharacter.IsValid()) // 检查转换是否成功且指针有效
	{
        // 如果 Owner 有效，则尝试获取其移动组件
		OwnerMovementComponent = OwnerEnemyCharacter->GetCharacterMovement();
        if(!OwnerMovementComponent.IsValid()) // 检查移动组件是否有效
        {
             UE_LOG(LogTemp, Warning, TEXT("EnemyAnimInstanceBase '%s': Could not get CharacterMovementComponent from Owner '%s'."),
                *GetNameSafe(this), *GetNameSafe(OwnerEnemyCharacter.Get()));
        }
	}
    else // 如果 Owning Actor 不是 AEnemyCharacterBase
    {
         UE_LOG(LogTemp, Error, TEXT("EnemyAnimInstanceBase '%s': Owning Actor is not derived from AEnemyCharacterBase! Casting failed."), *GetNameSafe(this));
    }
}

// OnTick_Implementation: 每帧更新函数
void UEnemyAnimInstanceBase::OnTick_Implementation(float DeltaTime)
{
	Super::OnTick_Implementation(DeltaTime); // 调用父类的 Tick 逻辑

	// --- 在 Tick 中只做最高频、最必要的状态更新 ---
    // 理想情况下，大部分状态应该由接口事件驱动。
    // 但像“是否在下落”这种纯物理状态，在 Tick 中检查通常更方便可靠。
	if (OwnerMovementComponent.IsValid())
	{
		bIsFalling = OwnerMovementComponent->IsFalling(); // 每帧检查是否在下落

        // Speed 和 bIsMoving 通常应该由 OnMovementStateChanged 接口更新，
        // 但如果需要更实时的物理速度，可以在这里获取：
        // Speed = OwnerMovementComponent->Velocity.Size2D();
        // bIsMoving = Speed > KINDA_SMALL_NUMBER;
	}
    else if(OwnerEnemyCharacter.IsValid()) // 如果移动组件指针失效了，尝试重新获取一次
    {
        // 这种情况可能发生在某些特殊的游戏流程中
         OwnerMovementComponent = OwnerEnemyCharacter->GetCharacterMovement();
         if(OwnerMovementComponent.IsValid())
         {
              bIsFalling = OwnerMovementComponent->IsFalling();
         }
         else // 如果还是获取不到，就重置状态
         {
             bIsFalling = true; // 假设在空中
             Speed = 0.f;
             bIsMoving = false;
         }
    }
     else // 如果连 Owner 都失效了
     {
         bIsFalling = true;
         Speed = 0.f;
         bIsMoving = false;
     }
}


// --- IEnemyAnimationStateListener 接口函数的具体实现 ---
// 这些函数会在 AI 或组件调用接口时被执行

void UEnemyAnimInstanceBase::OnMovementStateChanged_Implementation(float InSpeed, bool bInIsFalling, bool bInIsMoving)
{
	// 当收到移动状态更新通知时，更新内部变量
	// UE_LOG(LogTemp, Verbose, TEXT("EnemyAnimInstance '%s': OnMovementStateChanged - Speed: %.1f, Falling: %s, Moving: %s"),
    //      *GetNameSafe(this), InSpeed, bInIsFalling ? TEXT("True") : TEXT("False"), bInIsMoving ? TEXT("True") : TEXT("False"));

	this->Speed = InSpeed;
	this->bIsFalling = bInIsFalling; // 也接受接口传递的下落状态
    this->bIsMoving = bInIsMoving;
}

void UEnemyAnimInstanceBase::OnMeleeAttackStarted_Implementation(AActor* Target)
{
    // 当收到开始近战攻击的通知时
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyAnimInstance '%s': OnMeleeAttackStarted - Target: %s"),
    //      *GetNameSafe(this), Target ? *GetNameSafe(Target) : TEXT("None"));

	this->bIsAttackingMelee = true;  // 设置近战攻击状态为 true
    this->bIsAttackingRanged = false; // 确保与远程攻击状态互斥

    // 你可以在这里触发动画状态机的跳转，如果需要的话
    // 例如：JumpToNode(FName("MeleeAttackEntry")); // 跳转到名为 "MeleeAttackEntry" 的动画节点入口
}

void UEnemyAnimInstanceBase::OnRangedAttackStarted_Implementation(AActor* Target)
{
    // 当收到开始远程攻击的通知时
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyAnimInstance '%s': OnRangedAttackStarted - Target: %s"),
    //      *GetNameSafe(this), Target ? *GetNameSafe(Target) : TEXT("None"));

	this->bIsAttackingRanged = true; // 设置远程攻击状态为 true
    this->bIsAttackingMelee = false;  // 确保与近战攻击状态互斥

    // 可能触发跳转
    // JumpToNode(FName("RangedAttackEntry"));
}

void UEnemyAnimInstanceBase::OnDeathState_Implementation(AActor* Killer)
{
    // 当收到死亡通知时
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyAnimInstance '%s': OnDeathState - Killed by: %s"),
    //      *GetNameSafe(this), Killer ? *GetNameSafe(Killer) : TEXT("None"));

    this->bIsDead = true; // 设置死亡状态为 true

    // 同时重置其他可能的状态，因为死亡了通常就不会再攻击或移动了
    this->bIsAttackingMelee = false;
    this->bIsAttackingRanged = false;
    this->bIsMoving = false;
    this->Speed = 0.0f;

    // 可能触发跳转到死亡动画
    // JumpToNode(FName("DeathEntry"));
}

// 如果你在 IEnemyAnimationStateListener 接口中添加了其他函数，
// 别忘了在这里实现对应的 _Implementation 函数！
// 例如：
// void UEnemyAnimInstanceBase::OnStunStateChanged_Implementation(bool bIsStunned)
// {
//     // 更新眩晕状态变量...
// }