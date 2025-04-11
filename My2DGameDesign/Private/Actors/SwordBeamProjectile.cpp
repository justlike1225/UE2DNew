#include "Actors/SwordBeamProjectile.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

ASwordBeamProjectile::ASwordBeamProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->SetBoxExtent(FVector(16.0f, 8.0f, 8.0f));
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASwordBeamProjectile::OnCollisionOverlapBegin);

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("ProjectileSprite"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetCollisionProfileName(TEXT("NoCollision"));

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
}

void ASwordBeamProjectile::BeginPlay() { Super::BeginPlay(); }

void ASwordBeamProjectile::InitializeProjectile(const FVector& Direction, float Speed, float Damage, float LifeSpan,
                                                AActor* Shooter)
{
	InstigatorActor = Shooter;
	CurrentDamage = Damage;

	if (ProjectileMovement)
	{
		FVector NormalizedDirection = Direction.GetSafeNormal();
		ProjectileMovement->Velocity = NormalizedDirection * Speed;
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed > 0 ? Speed : 3000.0f;
	}

	if (LifeSpan > 0) { SetLifeSpan(LifeSpan); }

	if (Shooter && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(Shooter, true);
		TArray<UPrimitiveComponent*> Components;
		Shooter->GetComponents<UPrimitiveComponent>(Components);
		for (UPrimitiveComponent* Comp : Components) { CollisionComponent->IgnoreComponentWhenMoving(Comp, true); }
	}
}

void ASwordBeamProjectile::OnCollisionOverlapBegin(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor != InstigatorActor.Get())
	{
		AController* InstigatorController = InstigatorActor.IsValid()
			                                    ? InstigatorActor->GetInstigatorController()
			                                    : nullptr;

		UGameplayStatics::ApplyPointDamage(
			OtherActor, CurrentDamage, ProjectileMovement->Velocity.GetSafeNormal(), SweepResult,
			InstigatorController, this, UDamageType::StaticClass()
		);

		Destroy();
	}
}
