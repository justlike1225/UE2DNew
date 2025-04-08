// My2DGameDesign/Private/Components/EnemyRangedAttackComponent.cpp

#include "Components/EnemyRangedAttackComponent.h" // 引入头文件
#include "DataAssets/Enemy/EnemyRangedAttackSettingsDA.h" // 引入数据资产
#include "Actors/EnemyProjectileBase.h"           // 引入投掷物基类
#include "Enemies/EnemyCharacterBase.h"           // 引入敌人基类
#include "Interfaces/EnemyAnimationStateListener.h" // 引入动画监听器
#include "Interfaces/EnemyAnimationStateProvider.h" // 引入动画提供者
#include "TimerManager.h"                         // 定时器
#include "Engine/World.h"                         // GetWorld()
#include "GameFramework/Actor.h"                  // AActor
#include "GameFramework/Controller.h"           // AController
#include "Kismet/KismetMathLibrary.h"             // 需要用到 FindLookAtRotation 等数学函数


// 构造函数
UEnemyRangedAttackComponent::UEnemyRangedAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // 不需要 Tick
	SetIsReplicatedByDefault(false);          // 逻辑通常在服务器

	// 初始化状态
	bCanAttack = true;
	bIsAttacking = false;
}

// BeginPlay: 初始化
void UEnemyRangedAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取 Owner 引用
	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemyCharacter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyRangedAttackComponent '%s' requires its owner to be derived from AEnemyCharacterBase!"), *GetName());
		return;
	}

    // 尝试缓存动画监听器
    TryCacheAnimListener();

    // 检查数据资产和其中的关键配置（投掷物类）
    if(!AttackSettings)
    {
         UE_LOG(LogTemp, Warning, TEXT("EnemyRangedAttackComponent '%s' on Actor '%s' is missing AttackSettings Data Asset!"),
             *GetName(), *OwnerEnemyCharacter->GetName());
    }
    else if (!AttackSettings->ProjectileClass) // 检查是否设置了投掷物类
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyRangedAttackComponent '%s': AttackSettings DA ('%s') does not have ProjectileClass specified! Ranged attack will fail."),
            *GetName(), *AttackSettings->GetName());
    }
}

// EndPlay: 清理定时器
void UEnemyRangedAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if(GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
    }
	Super::EndPlay(EndPlayReason);
}


// ExecuteAttack: 由 AI 调用执行攻击
bool UEnemyRangedAttackComponent::ExecuteAttack(AActor* Target)
{
	// --- 前置条件检查 ---
    // 1. 能否攻击 (冷却)？ 2. 是否已在攻击中？ 3. 目标有效？ 4. 配置有效？ 5. 配置中的投掷物类有效？ 6. Owner有效？
	if (!bCanAttack || bIsAttacking || !Target || !AttackSettings || !AttackSettings->ProjectileClass || !OwnerEnemyCharacter.IsValid())
	{
        // （可选）记录失败原因的日志
		return false;
	}

    // (可选) 检查距离是否在配置的最小/最大范围内
    // float DistanceSq = FVector::DistSquared(OwnerEnemyCharacter->GetActorLocation(), Target->GetActorLocation());
    // float MinRangeSq = FMath::Square(AttackSettings->AttackRangeMin);
    // float MaxRangeSq = FMath::Square(AttackSettings->AttackRangeMax);
    // if (DistanceSq < MinRangeSq || DistanceSq > MaxRangeSq)
    // {
    //     UE_LOG(LogTemp, Verbose, TEXT("EnemyRangedAttackComponent '%s': Target '%s' is out of range (%.1f units). Attack cancelled."), *GetName(), *Target->GetName(), FMath::Sqrt(DistanceSq));
    //     return false;
    // }


    UE_LOG(LogTemp, Log, TEXT("EnemyRangedAttackComponent '%s' on '%s': Executing Ranged Attack on '%s'."),
        *GetName(), *OwnerEnemyCharacter->GetName(), *Target->GetName());

	// --- 开始攻击流程 ---
	bIsAttacking = true;
    bCanAttack = false;
	CurrentTarget = Target; // 存储目标

	// 启动冷却
	StartAttackCooldown();

	// 尝试通知动画实例播放动画
    if (!AnimationStateListener.GetInterface())
    {
        TryCacheAnimListener();
    }

	if (AnimationStateListener)
	{
		IEnemyAnimationStateListener::Execute_OnRangedAttackStarted(AnimationStateListener.GetObject(), Target);
        UE_LOG(LogTemp, Verbose, TEXT("EnemyRangedAttackComponent: Notified Animation Listener to start ranged attack."), *GetName());
	}
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("EnemyRangedAttackComponent '%s': Cannot notify Animation Listener - Interface is invalid."), *GetName());
    }

	return true;
}

// HandleSpawnProjectile: 由 AnimNotify 调用生成投掷物
void UEnemyRangedAttackComponent::HandleSpawnProjectile()
{
    // --- 安全检查 ---
    // 1. 是否处于攻击状态？ 2. 目标是否有效？ 3. 配置及投掷物类是否有效？ 4. Owner 是否有效？ 5. 是否能获取到 World？
	if (!bIsAttacking || !CurrentTarget.IsValid() || !AttackSettings || !AttackSettings->ProjectileClass || !OwnerEnemyCharacter.IsValid() || !GetWorld())
	{
        UE_LOG(LogTemp, Warning, TEXT("EnemyRangedAttackComponent '%s': HandleSpawnProjectile checks failed. Aborting spawn."), *GetName());
		return;
	}

    UE_LOG(LogTemp, Log, TEXT("EnemyRangedAttackComponent '%s': AnimNotify triggered HandleSpawnProjectile targeting '%s'."),
        *GetName(), *CurrentTarget->GetName());

    // --- 计算生成位置和旋转 ---
    FVector SpawnLocation;
    FRotator SpawnRotation;
    if (!CalculateSpawnTransform(SpawnLocation, SpawnRotation))
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyRangedAttackComponent '%s': Failed to calculate spawn transform. Aborting spawn."), *GetName());
        return; // 如果无法计算变换，则不生成
    }

    // --- 准备生成参数 ---
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerEnemyCharacter.Get(); // 发射者是敌人自己
    SpawnParams.Instigator = OwnerEnemyCharacter.Get(); // Instigator 也是敌人自己
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // 总是生成，即使位置有重叠

    // --- 生成投掷物 Actor ---
    AEnemyProjectileBase* NewProjectile = GetWorld()->SpawnActor<AEnemyProjectileBase>(
        AttackSettings->ProjectileClass, // 使用数据资产中配置的类
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    // --- 初始化生成的投掷物 ---
    if (NewProjectile)
    {
        UE_LOG(LogTemp, Verbose, TEXT("EnemyRangedAttackComponent: Successfully spawned projectile '%s'."), *NewProjectile->GetName());
        // 从数据资产获取参数
        float Speed = AttackSettings->ProjectileSpeed;
        float Damage = AttackSettings->AttackDamage;
        float LifeSpan = AttackSettings->ProjectileLifeSpan;
        AActor* Shooter = OwnerEnemyCharacter.Get();
        AController* ShooterController = OwnerEnemyCharacter->GetController();
        // 计算发射方向 (从生成点指向目标)
        FVector Direction = (CurrentTarget->GetActorLocation() - SpawnLocation).GetSafeNormal();

        // 调用投掷物的初始化函数
        NewProjectile->InitializeProjectile(Direction, Speed, Damage, LifeSpan, Shooter, ShooterController);

        // (可选) 在这里播放发射效果
        // if(AttackSettings->MuzzleEffect) { UGameplayStatics::SpawnEmitterAttached(...) }
        // if(AttackSettings->FireSound) { UGameplayStatics::PlaySoundAtLocation(...) }
    }
    else // 生成失败
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyRangedAttackComponent '%s': Failed to spawn projectile of class '%s'! Check configuration and world conditions."),
            *GetName(), *AttackSettings->ProjectileClass->GetName());
    }

    // 远程攻击通常在发射后就可以认为本次“攻击动作”的关键部分完成
    // 可以选择在这里或动画结束时重置 bIsAttacking，取决于你的设计
    // 如果在这里重置，AI 可以更快地进行下一个决策；如果等动画结束，行为更连贯
    // 我们目前的设计是在冷却结束后重置 bIsAttacking
}


// StartAttackCooldown: 启动冷却
void UEnemyRangedAttackComponent::StartAttackCooldown()
{
	if (AttackSettings && AttackSettings->AttackCooldown > 0)
	{
        // UE_LOG(LogTemp, Verbose, TEXT("EnemyRangedAttackComponent '%s': Starting cooldown for %.2f seconds."), *GetName(), AttackSettings->AttackCooldown);
		GetWorld()->GetTimerManager().SetTimer(
            AttackCooldownTimer,
            this,
            &UEnemyRangedAttackComponent::OnAttackCooldownFinished,
            AttackSettings->AttackCooldown,
            false
        );
	}
	else
	{
		OnAttackCooldownFinished(); // 立即结束冷却
	}
}

// OnAttackCooldownFinished: 冷却结束
void UEnemyRangedAttackComponent::OnAttackCooldownFinished()
{
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyRangedAttackComponent '%s': Cooldown finished."), *GetName());
	bCanAttack = true;
    bIsAttacking = false; // 在冷却结束后重置攻击状态
    CurrentTarget = nullptr;
}

// TryCacheAnimListener: 缓存动画监听器
bool UEnemyRangedAttackComponent::TryCacheAnimListener()
{
    // 与近战组件的实现完全相同
    if (!OwnerEnemyCharacter.IsValid()) return false;

    if(OwnerEnemyCharacter->GetClass()->ImplementsInterface(UEnemyAnimationStateProvider::StaticClass()))
    {
        AnimationStateListener = IEnemyAnimationStateProvider::Execute_GetEnemyAnimStateListener(OwnerEnemyCharacter.Get());
        if(AnimationStateListener.GetInterface())
        {
            return true;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("EnemyRangedAttackComponent '%s': Failed to cache Animation State Listener from owner '%s' (may not implement provider or listener)."),
        *GetName(), *OwnerEnemyCharacter->GetName());
    return false;
}

// CalculateSpawnTransform: 计算生成位置和旋转
bool UEnemyRangedAttackComponent::CalculateSpawnTransform(FVector& OutSpawnLocation, FRotator& OutSpawnRotation) const
{
    if (!OwnerEnemyCharacter.IsValid() || !CurrentTarget.IsValid() || !AttackSettings)
    {
        return false; // 无法计算所需信息
    }

    // 1. 获取基础位置和旋转
    // 可以使用 Owner 的位置，或者如果 Sprite 是根，用 Sprite 的位置
    FVector OwnerLocation = OwnerEnemyCharacter->GetActorLocation();
    FRotator OwnerRotation = OwnerEnemyCharacter->GetActorRotation(); // 当前朝向

    // 2. 处理偏移量
    FVector Offset = AttackSettings->SpawnOffset;
    // 考虑敌人左右朝向 (假设有 bIsFacingRight 状态或 GetFacingDirection)
    FVector FacingDirection = OwnerEnemyCharacter->GetFacingDirection(); // 使用我们之前实现的接口
    if (FacingDirection.X < 0) // 如果朝左
    {
        Offset.X *= -1.0f; // X 轴偏移反向
        // 注意：如果你的偏移量包含 Y 或 Z，可能也需要根据逻辑调整
    }
    // 将本地偏移量转换到世界空间
    FVector WorldOffset = OwnerRotation.RotateVector(Offset);

    // 3. 计算最终生成位置
    OutSpawnLocation = OwnerLocation + WorldOffset;

    // 4. 计算朝向目标的旋转
    FVector DirectionToTarget = (CurrentTarget->GetActorLocation() - OutSpawnLocation).GetSafeNormal();
    OutSpawnRotation = UKismetMathLibrary::FindLookAtRotation(OutSpawnLocation, CurrentTarget->GetActorLocation());
    // 或者直接使用方向向量计算: OutSpawnRotation = DirectionToTarget.Rotation();

    return true; // 计算成功
}