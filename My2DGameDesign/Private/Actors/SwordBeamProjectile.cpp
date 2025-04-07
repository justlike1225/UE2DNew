// My2DGameDesign/Private/Actors/SwordBeamProjectile.cpp

#include "Actors/SwordBeamProjectile.h"
#include "Components/BoxComponent.h" // 修改为 BoxComponent
#include "PaperSpriteComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

ASwordBeamProjectile::ASwordBeamProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox")); // 修改为 Box
    RootComponent = CollisionComponent;
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    // 设置合适的盒子大小，例如：
    CollisionComponent->SetBoxExtent(FVector(16.0f, 8.0f, 8.0f)); // 根据你的资源调整
    CollisionComponent->SetGenerateOverlapEvents(true);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASwordBeamProjectile::OnCollisionOverlapBegin);

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("ProjectileSprite"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetCollisionProfileName(TEXT("NoCollision"));

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = RootComponent;
    // 构造函数中设置的初始速度可能不再重要，因为它会被 InitializeProjectile 覆盖
    ProjectileMovement->InitialSpeed = 0.0f; // 可以设为0
    ProjectileMovement->MaxSpeed = 3000.0f; // 可以设置一个上限
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
}

void ASwordBeamProjectile::BeginPlay() { Super::BeginPlay(); }

// *** 实现新的 InitializeProjectile 函数 ***
void ASwordBeamProjectile::InitializeProjectile(const FVector& Direction, float Speed, float Damage, float LifeSpan, AActor* Shooter)
{
    InstigatorActor = Shooter;
    CurrentDamage = Damage; // 存储伤害值

    if (ProjectileMovement)
    {
        FVector NormalizedDirection = Direction.GetSafeNormal();
        ProjectileMovement->Velocity = NormalizedDirection * Speed;
        ProjectileMovement->InitialSpeed = Speed; // 确保设置了 InitialSpeed
        ProjectileMovement->MaxSpeed = Speed > 0 ? Speed : 3000.0f; // 防止 MaxSpeed 为0，可以给个上限
        UE_LOG(LogTemp, Log, TEXT("Projectile '%s' Initialized. Speed: %.2f, Damage: %.1f, LifeSpan: %.2f"),
               *GetName(), Speed, CurrentDamage, LifeSpan);
    }

    // *** 使用传入的 LifeSpan 设置生命周期 ***
    if (LifeSpan > 0) { SetLifeSpan(LifeSpan); }

    // 忽略发射者碰撞
     if (Shooter && CollisionComponent)
     {
          CollisionComponent->IgnoreActorWhenMoving(Shooter, true);
          TArray<UPrimitiveComponent*> Components;
          Shooter->GetComponents<UPrimitiveComponent>(Components);
          for(UPrimitiveComponent* Comp : Components) { CollisionComponent->IgnoreComponentWhenMoving(Comp, true); }
     }
}

// OnCollisionOverlapBegin: 使用存储的伤害值
void ASwordBeamProjectile::OnCollisionOverlapBegin(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherActor != InstigatorActor.Get())
    {
        AController* InstigatorController = InstigatorActor.IsValid() ? InstigatorActor->GetInstigatorController() : nullptr;

        // *** 使用存储的 CurrentDamage ***
        UGameplayStatics::ApplyPointDamage(
            OtherActor, CurrentDamage, ProjectileMovement->Velocity.GetSafeNormal(), SweepResult,
            InstigatorController, this, UDamageType::StaticClass()
        );

        UE_LOG(LogTemp, Log, TEXT("Projectile '%s' hit %s for %.1f damage."), *GetName(), *OtherActor->GetName(), CurrentDamage);

        Destroy(); // 命中后销毁
    }
}