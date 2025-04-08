// My2DGameDesign/Private/Components/EnemyMeleeAttackComponent.cpp

#include "Components/EnemyMeleeAttackComponent.h" // 引入头文件
#include "DataAssets/Enemy/EnemyMeleeAttackSettingsDA.h" // 引入数据资产头文件
#include "Enemies/EnemyCharacterBase.h"              // 引入敌人基类头文件
#include "Interfaces/EnemyAnimationStateListener.h"  // 引入动画监听器接口
#include "Interfaces/Damageable.h"                   // 引入可受击接口
#include "TimerManager.h"                            // 用于设置和清除定时器
#include "Engine/World.h"                            // 需要 GetWorld()
#include "GameFramework/Controller.h"              // 需要获取 Controller

// 构造函数
UEnemyMeleeAttackComponent::UEnemyMeleeAttackComponent()
{
	// 设置组件可在游戏开始时运行 BeginPlay
	PrimaryComponentTick.bCanEverTick = false; // 近战攻击组件通常不需要每帧 Tick
	SetIsReplicatedByDefault(false); // 敌人攻击逻辑通常在服务器执行，组件状态同步可能不需要（除非有特殊需求）

	// 初始化状态
	bCanAttack = true;
	bIsAttacking = false;
    bHasAppliedDamageThisAttack = false;
}

// BeginPlay: 初始化
void UEnemyMeleeAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取并缓存 Owner 引用
	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemyCharacter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyMeleeAttackComponent '%s' requires its owner to be derived from AEnemyCharacterBase!"), *GetName());
		// 如果 Owner 无效，这个组件就无法工作，可以选择禁用它
        // SetActive(false);
        // SetComponentTickEnabled(false);
		return;
	}

    // 尝试缓存动画监听器接口
    TryCacheAnimListener();

    // 检查数据资产是否已配置
    if(!AttackSettings)
    {
         UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent '%s' on Actor '%s' is missing AttackSettings Data Asset! Attack will likely fail."),
             *GetName(), *OwnerEnemyCharacter->GetName());
    }
}

// EndPlay: 清理定时器
void UEnemyMeleeAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清除可能仍在运行的冷却计时器，防止内存泄漏或意外行为
    if(GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
    }
	Super::EndPlay(EndPlayReason);
}


// ExecuteAttack: 由 AI 调用执行攻击
bool UEnemyMeleeAttackComponent::ExecuteAttack(AActor* Target)
{
	
	if (!bCanAttack || bIsAttacking || !Target || !AttackSettings || !OwnerEnemyCharacter.IsValid())
	{
        
		return false; // 攻击未能开始
	}

    UE_LOG(LogTemp, Log, TEXT("EnemyMeleeAttackComponent '%s' on '%s': Executing Melee Attack on '%s'."),
        *GetName(), *OwnerEnemyCharacter->GetName(), *Target->GetName());


	// --- 开始攻击流程 ---
	bIsAttacking = true;         // 标记为正在攻击
    bCanAttack = false;          // 标记为不能立即再次攻击 (进入冷却)
    bHasAppliedDamageThisAttack = false; // 重置本次攻击的伤害标记
	CurrentTarget = Target;      // 存储当前攻击目标

	// 启动攻击冷却计时器
	StartAttackCooldown();

	// 尝试通知动画实例播放攻击动画
    if (!AnimationStateListener.GetInterface()) // 如果接口无效，尝试再次获取
    {
        TryCacheAnimListener();
    }

	if (AnimationStateListener) // 检查接口是否有效
	{
		// 通过接口调用动画实例的函数
		IEnemyAnimationStateListener::Execute_OnMeleeAttackStarted(AnimationStateListener.GetObject(), Target);
         UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Notified Animation Listener to start melee attack."), *GetName());
	}
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent '%s': Cannot notify Animation Listener - Interface is invalid."), *GetName());
         // 没有动画实例监听，攻击动画可能不会播放，但伤害逻辑仍然可以执行（如果需要）
    }


	return true; // 攻击成功开始
}

// HandleDamageApplication: 由 AnimNotify 调用来施加伤害
void UEnemyMeleeAttackComponent::HandleDamageApplication()
{
	// --- 安全检查 ---
    // 1. 是否处于攻击状态中? (防止非攻击状态下误触发)
    // 2. 本次攻击是否已经施加过伤害了? (防止重复伤害)
    // 3. 当前目标是否还有效? (可能在动画播放过程中目标死亡或消失)
    // 4. 配置和 Owner 是否有效?
	if (!bIsAttacking || bHasAppliedDamageThisAttack || !CurrentTarget.IsValid() || !AttackSettings || !OwnerEnemyCharacter.IsValid())
	{
        // UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent '%s': HandleDamageApplication called but checks failed (IsAttacking:%s, HasAppliedDamage:%s, TargetValid:%s, Settings:%s, Owner:%s)"),
        //     *GetName(), bIsAttacking?TEXT("T"):TEXT("F"), bHasAppliedDamageThisAttack?TEXT("T"):TEXT("F"), CurrentTarget.IsValid()?TEXT("T"):TEXT("F"), AttackSettings?TEXT("T"):TEXT("F"), OwnerEnemyCharacter.IsValid()?TEXT("T"):TEXT("F"));
		return;
	}

    UE_LOG(LogTemp, Log, TEXT("EnemyMeleeAttackComponent '%s': AnimNotify triggered HandleDamageApplication on target '%s'."),
        *GetName(), *CurrentTarget->GetName());

	// --- 执行伤害逻辑 ---
	AActor* TargetActor = CurrentTarget.Get(); // 获取目标 Actor 的强指针
    AActor* DamageCauser = OwnerEnemyCharacter.Get(); // 伤害来源是敌人自己
    AController* InstigatorController = OwnerEnemyCharacter->GetController(); // 获取敌人的控制器

    // 检查目标是否实现了 IDamageable 接口
    if (TargetActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
    {
        // 从数据资产获取伤害值
        float DamageToApply = AttackSettings->AttackDamage;

        // 通过接口调用目标的 ApplyDamage 函数
        IDamageable::Execute_ApplyDamage(TargetActor, DamageToApply, DamageCauser, InstigatorController, FHitResult()); // 最后参数是空的 HitResult
        UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Applied %.1f damage to '%s' via IDamageable interface."), DamageToApply, *TargetActor->GetName());

        // 标记本次攻击已成功施加伤害
        bHasAppliedDamageThisAttack = true;

        // (可选) 在这里播放命中特效和声音
        // if(AttackSettings->HitEffect) { UGameplayStatics::SpawnEmitterAtLocation(...) }
        // if(AttackSettings->HitSound) { UGameplayStatics::PlaySoundAtLocation(...) }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent '%s': Target '%s' does not implement IDamageable interface. No damage applied."),
            *GetName(), *TargetActor->GetName());
    }

    // 注意：伤害判定范围的检查可以放在这里，例如：
    // float DistanceSq = FVector::DistSquared(DamageCauser->GetActorLocation(), TargetActor->GetActorLocation());
    // float AttackRangeSq = FMath::Square(AttackSettings->AttackRange * 1.1f); // 可以给范围一点容错
    // if (DistanceSq <= AttackRangeSq) { ... 应用伤害 ... }
    // 但通常依赖 AnimNotify 触发时目标就在范围内更简单。
}

// StartAttackCooldown: 启动冷却计时器
void UEnemyMeleeAttackComponent::StartAttackCooldown()
{
	if (AttackSettings && AttackSettings->AttackCooldown > 0)
	{
        // UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent '%s': Starting cooldown for %.2f seconds."), *GetName(), AttackSettings->AttackCooldown);
		GetWorld()->GetTimerManager().SetTimer(
            AttackCooldownTimer, // 定时器句柄
            this,                           // 调用函数的对象
            &UEnemyMeleeAttackComponent::OnAttackCooldownFinished, // 要调用的函数
            AttackSettings->AttackCooldown, // 延迟时间（从数据资产读取）
            false                           // false 表示不循环
        );
	}
	else // 如果没有设置冷却时间，则立即完成冷却（相当于可以连续攻击）
	{
		OnAttackCooldownFinished();
	}
}

// OnAttackCooldownFinished: 冷却结束时调用
void UEnemyMeleeAttackComponent::OnAttackCooldownFinished()
{
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent '%s': Cooldown finished."), *GetName());
	bCanAttack = true;   // 标记为可以再次攻击
    bIsAttacking = false; // 标记攻击状态结束
    CurrentTarget = nullptr; // 清除当前目标
    // bHasAppliedDamageThisAttack 会在下次 ExecuteAttack 时重置
}

// TryCacheAnimListener: 尝试获取并缓存动画监听器
bool UEnemyMeleeAttackComponent::TryCacheAnimListener()
{
	if (!OwnerEnemyCharacter.IsValid()) return false;

	// 检查 Owner 是否实现了 *敌人* 的 Provider 接口
	if(OwnerEnemyCharacter->GetClass()->ImplementsInterface(UEnemyAnimationStateProvider::StaticClass()))
	{
		// 通过 *敌人* Provider 接口获取 *敌人* Listener 接口
		AnimationStateListener = IEnemyAnimationStateProvider::Execute_GetEnemyAnimStateListener(OwnerEnemyCharacter.Get()); // <--- 调用新的 Provider 函数
		if(AnimationStateListener.GetInterface())
		{
			UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent '%s': Successfully cached *Enemy* Animation State Listener."), *GetName());
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent '%s': Owner implements Provider but returned invalid *Enemy* Listener interface."), *GetName());
			return false;
		}
	}
	else // Owner 没有实现敌人 Provider 接口
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent '%s': Owner '%s' does not implement IEnemyAnimationStateProvider interface."),
		   *GetName(), *OwnerEnemyCharacter->GetName());
		return false;
	}
}