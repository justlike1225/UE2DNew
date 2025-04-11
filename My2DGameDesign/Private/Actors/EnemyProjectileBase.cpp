// My2DGameDesign/Private/Actors/EnemyProjectileBase.cpp

#include "Actors/EnemyProjectileBase.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interfaces/Damageable.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"

AEnemyProjectileBase::AEnemyProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 0.0f;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitSphereRadius(16.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemyProjectileBase::OnProjectileOverlapBegin);

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("ProjectileSprite"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetCollisionProfileName(TEXT("NoCollision"));
	SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->UpdatedComponent = RootComponent;
	ProjectileMovementComponent->InitialSpeed = 0.f;
	ProjectileMovementComponent->MaxSpeed = 3000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AEnemyProjectileBase::InitializeProjectile(const FVector& Direction, float Speed, float Damage, float LifeSpan,
                                                AActor* Shooter, AController* ShooterController)
{
	CurrentDamage = Damage;
	InstigatorActor = Shooter;
	InstigatorController = ShooterController;

	if (LifeSpan > 0)
	{
		SetLifeSpan(LifeSpan);
	}

	if (ProjectileMovementComponent)
	{
		FVector SafeDirection = Direction.GetSafeNormal();
		ProjectileMovementComponent->Velocity = SafeDirection * Speed;
		ProjectileMovementComponent->InitialSpeed = Speed;
		ProjectileMovementComponent->MaxSpeed = FMath::Max(Speed, ProjectileMovementComponent->MaxSpeed);
	}

	if (Shooter && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(Shooter, true);
		TArray<UPrimitiveComponent*> ComponentsToIgnore;
		Shooter->GetComponents<UPrimitiveComponent>(ComponentsToIgnore);
		for (UPrimitiveComponent* Comp : ComponentsToIgnore)
		{
			if (Comp && Comp->IsCollisionEnabled())
			{
				CollisionComponent->IgnoreComponentWhenMoving(Comp, true);
			}
		}
	}
}

void AEnemyProjectileBase::OnProjectileOverlapBegin_Implementation(UPrimitiveComponent* OverlappedComponent,
                                                                   AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                                                   int32 OtherBodyIndex, bool bFromSweep,
                                                                   const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == InstigatorActor.Get())
	{
		return;
	}

	if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		AController* EventInstigator = InstigatorController.Get();
		IDamageable::Execute_ApplyDamage(OtherActor, CurrentDamage, InstigatorActor.Get(), EventInstigator,
		                                 SweepResult);
		Destroy();
	}
	else
	{
		if (OtherComp && OtherComp->IsSimulatingPhysics() || OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
		{
			Destroy();
		}
	}
}
