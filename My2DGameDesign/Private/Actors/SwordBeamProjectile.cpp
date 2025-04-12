#include "Actors/SwordBeamProjectile.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interfaces/Damageable.h"
#include "GameFramework/Controller.h"

ASwordBeamProjectile::ASwordBeamProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));


	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASwordBeamProjectile::OnCollisionOverlapBegin);

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("ProjectileSprite"));


	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

void ASwordBeamProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 可以在这里设置初始生命周期，或者完全依赖 InitializeProjectile
	// InitialLifeSpan = 3.0f;
}

void ASwordBeamProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CollisionComponent)
	{
		RootComponent = CollisionComponent;
		CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		CollisionComponent->SetBoxExtent(FVector(16.0f, 8.0f, 8.0f));
		CollisionComponent->SetGenerateOverlapEvents(true);
	}
    if (SpriteComponent)
    {
	    SpriteComponent->SetupAttachment(RootComponent);
    	SpriteComponent->SetCollisionProfileName(TEXT("NoCollision"));
    }
	if (ProjectileMovement)
	{
		ProjectileMovement->UpdatedComponent = RootComponent;
		ProjectileMovement->InitialSpeed = 0.0f;
		ProjectileMovement->MaxSpeed = 3000.0f;
		ProjectileMovement->bRotationFollowsVelocity = true;
		ProjectileMovement->bShouldBounce = false;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
	}
}

// InitializeProjectile 函数保持不变
void ASwordBeamProjectile::InitializeProjectile(const FVector& Direction, float Speed, float Damage, float LifeSpan,
                                                AActor* Shooter)
{
	InstigatorActor = Shooter; // 存储发射者 Actor 的弱引用
	CurrentDamage = Damage; // 存储伤害值

	if (ProjectileMovement)
	{
		FVector NormalizedDirection = Direction.GetSafeNormal(); // 确保方向向量是单位向量
		ProjectileMovement->Velocity = NormalizedDirection * Speed;
		ProjectileMovement->InitialSpeed = Speed;
		// 确保最大速度至少为初始速度
		ProjectileMovement->MaxSpeed = FMath::Max(Speed, ProjectileMovement->MaxSpeed);
	}

	// 设置生命周期，如果大于 0
	if (LifeSpan > 0)
	{
		SetLifeSpan(LifeSpan);
	}

	// 忽略发射者自身的碰撞
	if (Shooter && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(Shooter, true);
		// （可选）更彻底地忽略发射者的所有碰撞组件
		TArray<UPrimitiveComponent*> ComponentsToIgnore;
		Shooter->GetComponents<UPrimitiveComponent>(ComponentsToIgnore);
		for (UPrimitiveComponent* Comp : ComponentsToIgnore)
		{
			if (Comp && Comp->IsCollisionEnabled()) // 只忽略启用了碰撞的组件
			{
				CollisionComponent->IgnoreComponentWhenMoving(Comp, true);
			}
		}
	}
}

// --- 修改后的碰撞处理函数 ---
void ASwordBeamProjectile::OnCollisionOverlapBegin(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 1. 基本检查：确保 OtherActor 有效，不是抛射物自身，也不是发射者
	if (!OtherActor || OtherActor == this || OtherActor == InstigatorActor.Get())
	{
		return; // 忽略这些情况
	}

	// 2. 检查 OtherActor 是否实现了 IDamageable 接口
	// 使用 UDamageable::StaticClass() 获取接口的 UClass
	if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		// 3. 如果实现了接口，调用接口的 ApplyDamage 函数

		// 获取发射者的控制器 (可能需要更健壮的方式获取，比如从 InstigatorActor 获取 Pawn 再获取 Controller)
		AController* InstigatorController = nullptr;
		if (InstigatorActor.IsValid()) // 检查弱引用是否仍然有效
		{
			// 尝试将发射者转换为 APawn 以获取 Controller
			APawn* InstigatorPawn = Cast<APawn>(InstigatorActor.Get());
			if (InstigatorPawn)
			{
				InstigatorController = InstigatorPawn->GetController();
			}
		}

		// 调用 IDamageable 接口的 ApplyDamage 函数
		// 参数：目标Actor, 伤害值, 伤害来源Actor, 来源控制器, HitResult
		IDamageable::Execute_ApplyDamage(OtherActor, CurrentDamage, InstigatorActor.Get(), InstigatorController,
		                                 SweepResult);

		// 4. 造成伤害后销毁抛射物
		UE_LOG(LogTemp, Log, TEXT("SwordBeamProjectile hit Damageable Actor: %s. Applying damage and destroying."),
		       *OtherActor->GetName());
		Destroy();
	}
	else
	{
		// 5. 如果 OtherActor 没有实现 IDamageable 接口（比如撞墙、撞到非生命物体）
		// 我们可以选择销毁抛射物，或者让它继续飞行，或者根据 OtherComp 的类型判断。
		// 这里我们采用类似 AEnemyProjectileBase 的逻辑：如果撞到模拟物理的物体或静态世界物体，则销毁。
		if (OtherComp && (OtherComp->IsSimulatingPhysics() || OtherComp->GetCollisionObjectType() == ECC_WorldStatic))
		{
			UE_LOG(LogTemp, Log, TEXT("SwordBeamProjectile hit non-damageable static/physics object: %s. Destroying."),
			       *OtherActor->GetName());
			Destroy();
		}
	}
}
