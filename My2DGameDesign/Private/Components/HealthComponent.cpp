// My2DGameDesign/Private/Components/HealthComponent.cpp

#include "Components/HealthComponent.h" // 引入头文件
#include "GameFramework/Actor.h"       // 需要 GetOwner() 来获取拥有此组件的 Actor
#include "Engine/World.h"             // 可能需要 GetWorld() (虽然在此实现中暂时没用到)
#include "GameFramework/Controller.h"  // 需要 Controller 类型
#include "GameFramework/DamageType.h"  // 虽然没直接用，但伤害系统通常会涉及 (用于 HandleOwnerTakeAnyDamage)

// 构造函数
UHealthComponent::UHealthComponent()
{
	// 设置组件可以在游戏开始时运行 BeginPlay
	PrimaryComponentTick.bCanEverTick = false; // 生命组件通常不需要每帧更新，设为 false 以优化性能
	SetIsReplicatedByDefault(true); // 如果是网络游戏，生命值通常需要网络同步，这里先标记上

    // 初始化内部状态变量
    MaxHealth = DefaultMaxHealth; // 使用默认值初始化 MaxHealth
    CurrentHealth = MaxHealth;    // 开始时满血
    bIsDead = false;              // 开始时是活着的
}

// BeginPlay 函数：在游戏开始时进行初始化
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay(); // 调用父类的 BeginPlay

	// 确保运行时使用的 MaxHealth 与 DefaultMaxHealth 一致
	MaxHealth = DefaultMaxHealth;
	// 设置当前生命值为最大值
	CurrentHealth = MaxHealth;
	// 重置死亡状态
	bIsDead = false;

    // 首次广播当前的生命值状态，让UI等系统可以获取初始值
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    // --- 绑定到引擎伤害系统 (可选) ---
    // 如果你想让这个组件响应 Actor 的 TakeDamage 调用 (比如 ApplyDamage API)，
    // 就需要在这里取消注释并确保 Damageable 接口被正确实现。
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
        // 绑定到 Actor 的 OnTakeAnyDamage 委托。这样，任何对 Owner Actor 调用 TakeDamage 的操作，
        // 最终都会触发我们下面的 HandleOwnerTakeAnyDamage 函数 (如果取消注释的话)。
		// MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleOwnerTakeAnyDamage);
	}
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HealthComponent created for a null Owner!"));
    }
}

// 处理伤害的核心逻辑
float UHealthComponent::TakeDamage(float DamageAmount, AActor* DamageCauser, AController* InstigatorController)
{
    // 1. 检查是否可以接受伤害 (是否已死亡? 伤害是否有效?)
	if (bIsDead || DamageAmount <= 0.f)
	{
        // 如果已死亡或伤害无效，则不执行任何操作，返回 0 伤害
		return 0.f;
	}

	// 2. 计算实际生命值 (可以加入伤害减免、格挡等逻辑)
    // FMath::Max 确保生命值不会低于 0
	float HealthBeforeDamage = CurrentHealth;
	CurrentHealth = FMath::Max(CurrentHealth - DamageAmount, 0.0f);
    float ActualDamage = HealthBeforeDamage - CurrentHealth; // 计算实际扣除的血量

	// 3. 打印日志 (方便调试)
	UE_LOG(LogTemp, Log, TEXT("HealthComponent on '%s': Took %.1f damage from '%s'. Health %.1f -> %.1f"),
           GetOwner() ? *GetOwner()->GetName() : TEXT("UnknownOwner"), // 获取拥有者的名字
           ActualDamage,                                             // 打印实际伤害
           DamageCauser ? *DamageCauser->GetName() : TEXT("UnknownSource"), // 伤害来源的名字
		   HealthBeforeDamage, CurrentHealth);                            // 打印变化前后的生命值

	// 4. 广播生命值变化事件，通知其他系统（如UI更新血条）
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	// 5. 检查是否死亡
	if (CurrentHealth <= 0.f)
	{
        // 调用内部的死亡处理函数
        HandleDeath(DamageCauser);
	}

    // 6. 返回实际造成的伤害值
	return ActualDamage;
}

// 内部私有函数，处理死亡逻辑
void UHealthComponent::HandleDeath(AActor* Killer)
{
    // 防止重复执行死亡逻辑
    if (bIsDead) return;

    bIsDead = true; // 标记为已死亡
    UE_LOG(LogTemp, Log, TEXT("HealthComponent on '%s': Died. Killed by '%s'."),
           GetOwner() ? *GetOwner()->GetName() : TEXT("UnknownOwner"),
           Killer ? *Killer->GetName() : TEXT("Unknown Cause"));

    // 广播死亡事件，通知其他系统（如AI停止、播放死亡动画、掉落物品等）
    // 注意 Killer 可能是 nullptr
    OnDeath.Broadcast(Killer);

    // 这里可以添加一些通用的死亡处理，但通常具体行为由监听 OnDeath 的 Actor (如 AEnemyCharacterBase) 处理。
    // 例如，可以考虑禁用此组件，防止进一步的交互。
    // Deactivate(); // 可以禁用组件
}


// --- 处理引擎伤害系统的函数 (如果选择使用) ---
/*
// 如果在 BeginPlay 中绑定了 OnTakeAnyDamage，就需要实现这个函数
void UHealthComponent::HandleOwnerTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    // 直接调用我们自己的 TakeDamage 函数来处理
    TakeDamage(Damage, DamageCauser, InstigatedBy);
}
*/