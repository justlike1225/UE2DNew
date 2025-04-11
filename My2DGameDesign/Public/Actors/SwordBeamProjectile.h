#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwordBeamProjectile.generated.h"

class UBoxComponent;
class UPaperSpriteComponent;
class UProjectileMovementComponent;
class UGameplayStatics;
class UDamageType;

UCLASS()
class MY2DGAMEDESIGN_API ASwordBeamProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASwordBeamProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

protected:
	UPROPERTY()
	float CurrentDamage = 0.0f;
	UPROPERTY()
	TWeakObjectPtr<AActor> InstigatorActor;

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnCollisionOverlapBegin(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult
	);

public:
	void InitializeProjectile(
		const FVector& Direction,
		float Speed,
		float Damage,
		float LifeSpan,
		AActor* Shooter
	);
};
