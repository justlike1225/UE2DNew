// My2DGameDesign/Private/Actors/EnemyProjectileBase.cpp

#include "Actors/EnemyProjectileBase.h"       // 引入头文件
#include "Components/SphereComponent.h"         // 球形碰撞体
#include "PaperSpriteComponent.h"             // 2D精灵组件
#include "GameFramework/ProjectileMovementComponent.h" // 投掷物移动组件
#include "Interfaces/Damageable.h"               // 可受击接口
#include "Kismet/GameplayStatics.h"              // 需要用到 ApplyPointDamage (如果不用接口) 或其他静态函数
#include "GameFramework/Controller.h"          // Controller 基类

// 构造函数
AEnemyProjectileBase::AEnemyProjectileBase()
{
 	// 设置 Actor 基本属性
	PrimaryActorTick.bCanEverTick = false; // 投掷物通常不需要每帧 Tick，移动由组件处理
    InitialLifeSpan = 0.0f;              // 默认不自动销毁，由 InitializeProjectile 设置

	// --- 创建并设置碰撞组件 ---
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionComponent; // 将碰撞体设为根组件
	CollisionComponent->InitSphereRadius(16.0f); // 设置默认碰撞半径 (需要根据实际视觉调整)
	// 设置碰撞预设：
    // OverlapAllDynamic: 与所有动态物体产生重叠事件。
    // 你也可以创建自定义的碰撞通道和预设，例如 "EnemyProjectile"。
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 只进行查询（重叠），不进行物理模拟碰撞
    CollisionComponent->CanCharacterStepUpOn = ECB_No; // 角色不能踩在上面
    CollisionComponent->SetGenerateOverlapEvents(true); // 确保生成重叠事件

    // 绑定重叠事件处理函数
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemyProjectileBase::OnProjectileOverlapBegin);


	// --- 创建并设置视觉组件 (2D 精灵) ---
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("ProjectileSprite"));
	SpriteComponent->SetupAttachment(RootComponent); // 附加到碰撞体上
	SpriteComponent->SetCollisionProfileName(TEXT("NoCollision")); // 精灵本身不需要碰撞
	SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	// --- 创建并设置投掷物移动组件 ---
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->UpdatedComponent = RootComponent; // 移动组件作用于根组件（碰撞体）
	ProjectileMovementComponent->InitialSpeed = 0.f;     // 初始速度将在 InitializeProjectile 中设置
	ProjectileMovementComponent->MaxSpeed = 3000.0f;    // 设置一个最大速度限制
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // 让投掷物的朝向自动跟随速度方向
	ProjectileMovementComponent->bShouldBounce = false;  // 通常投掷物命中后不反弹
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f; // 默认为 0，即不受重力影响，直线飞行 (如果需要抛物线，可以设置为 > 0)

    // (可选) 绑定停止事件
    // ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &AEnemyProjectileBase::OnProjectileStop);
}

// InitializeProjectile: 初始化函数
void AEnemyProjectileBase::InitializeProjectile(const FVector& Direction, float Speed, float Damage, float LifeSpan, AActor* Shooter, AController* ShooterController)
{
    // --- 存储传入的参数 ---
    CurrentDamage = Damage;
    InstigatorActor = Shooter;         // 存储发射者 Actor 的弱指针
    InstigatorController = ShooterController; // 存储发射者 Controller 的弱指针

    UE_LOG(LogTemp, Log, TEXT("Projectile '%s' Initialized: Speed=%.1f, Damage=%.1f, LifeSpan=%.1f, Shooter='%s'"),
           *GetName(), Speed, Damage, LifeSpan, Shooter ? *Shooter->GetName() : TEXT("None"));

    // --- 设置投掷物生命周期 ---
    if (LifeSpan > 0)
    {
	    SetLifeSpan(LifeSpan); // 设置 Actor 在 N 秒后自动销毁
    }

	// --- 配置移动组件 ---
	if (ProjectileMovementComponent)
	{
        // 设置初始速度和方向
        FVector SafeDirection = Direction.GetSafeNormal(); // 确保方向是单位向量
		ProjectileMovementComponent->Velocity = SafeDirection * Speed;
        ProjectileMovementComponent->InitialSpeed = Speed; // 也设置一下 InitialSpeed
        ProjectileMovementComponent->MaxSpeed = FMath::Max(Speed, ProjectileMovementComponent->MaxSpeed); // 确保最大速度不小于初始速度

        UE_LOG(LogTemp, Verbose, TEXT("Projectile '%s': Velocity set to %s"), *GetName(), *ProjectileMovementComponent->Velocity.ToString());
	}
    else
    {
         UE_LOG(LogTemp, Error, TEXT("Projectile '%s': ProjectileMovementComponent is missing! Cannot set velocity."), *GetName());
    }


    // --- 设置碰撞忽略 ---
    // 确保投掷物不会与发射者自身发生碰撞
    if (Shooter && CollisionComponent)
    {
        CollisionComponent->IgnoreActorWhenMoving(Shooter, true);
        // 如果发射者有多个碰撞体，可能需要遍历并全部忽略
        TArray<UPrimitiveComponent*> ComponentsToIgnore;
        Shooter->GetComponents<UPrimitiveComponent>(ComponentsToIgnore);
        for(UPrimitiveComponent* Comp : ComponentsToIgnore)
        {
            if(Comp && Comp->IsCollisionEnabled()) // 只忽略启用了碰撞的组件
            {
                CollisionComponent->IgnoreComponentWhenMoving(Comp, true);
            }
        }
         UE_LOG(LogTemp, Verbose, TEXT("Projectile '%s': Ignoring collision with shooter '%s'."), *GetName(), *Shooter->GetName());
    }
}


// OnProjectileOverlapBegin_Implementation: 处理重叠事件的 C++ 实现
void AEnemyProjectileBase::OnProjectileOverlapBegin_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // --- 安全检查 ---
    // 1. OtherActor 是否有效?
    // 2. 是否是投掷物自身?
    // 3. 是否是发射者? (使用弱指针检查，更安全)
	if (!OtherActor || OtherActor == this || OtherActor == InstigatorActor.Get())
	{
        // 如果是自身或发射者，忽略此次重叠
		return;
	}

    UE_LOG(LogTemp, Log, TEXT("Projectile '%s': Overlapped with '%s'."), *GetName(), *OtherActor->GetName());

    // --- 尝试施加伤害 ---
    // 检查重叠的 Actor 是否实现了 IDamageable 接口
    if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
    {
        UE_LOG(LogTemp, Verbose, TEXT("Projectile '%s': Target '%s' implements IDamageable. Applying damage..."), *GetName(), *OtherActor->GetName());

        // 获取我们存储的发射者 Controller (可能是 AIController 或 PlayerController)
        AController* EventInstigator = InstigatorController.Get();
        // 施加伤害！调用目标的 ApplyDamage 接口函数
        // 注意传递伤害值、发射者 Actor、发射者 Controller
        IDamageable::Execute_ApplyDamage(OtherActor, CurrentDamage, InstigatorActor.Get(), EventInstigator, SweepResult);

        // --- 命中后的处理 ---
        // 播放命中效果 (可以在蓝图子类中实现，或在这里添加)
        // UGameplayStatics::SpawnEmitterAtLocation(...)
        // UGameplayStatics::PlaySoundAtLocation(...)

        // 销毁投掷物
        Destroy();
    }
    else // 如果目标不能被伤害
    {
         UE_LOG(LogTemp, Verbose, TEXT("Projectile '%s': Target '%s' does not implement IDamageable. No damage applied."), *GetName(), *OtherActor->GetName());
         // 根据游戏规则决定是否销毁。例如，撞到墙壁也应该销毁。
         // 如果 OtherComp (对方的碰撞体) 是 WorldStatic 或类似的类型，也可以销毁。
         if(OtherComp && OtherComp->IsSimulatingPhysics() || OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
         {
             // 撞到了物理对象或静态世界物体，销毁
             UE_LOG(LogTemp, Verbose, TEXT("Projectile '%s': Hit a non-damageable object. Destroying."), *GetName());
             Destroy();
         }
         // 否则，如果撞到的只是一个触发器或者其他不该销毁投掷物的东西，就让它继续飞。
    }
}

// (可选) 处理撞停事件
/*
void AEnemyProjectileBase::OnProjectileStop(const FHitResult& ImpactResult)
{
    UE_LOG(LogTemp, Log, TEXT("Projectile '%s' stopped due to impact."), *GetName());
    // 在这里可以播放撞击效果
    // UGameplayStatics::SpawnEmitterAtLocation(...)
    // UGameplayStatics::PlaySoundAtLocation(...)

    // 销毁自身
    Destroy();
}
*/