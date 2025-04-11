#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyProjectileBase.generated.h"

class USphereComponent;
class UPaperSpriteComponent;
class UProjectileMovementComponent;
class AController;

UCLASS(Abstract, Blueprintable)
class MY2DGAMEDESIGN_API AEnemyProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AEnemyProjectileBase();

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void InitializeProjectile(const FVector& Direction, float Speed, float Damage, float LifeSpan,
	                                  AActor* Shooter, AController* ShooterController);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Projectile | Runtime",
		meta = (AllowPrivateAccess = "true"))
	float CurrentDamage = 0.0f;

	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Projectile | Runtime",
		meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AActor> InstigatorActor;

	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Projectile | Runtime",
		meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AController> InstigatorController;

	UFUNCTION(BlueprintNativeEvent, Category = "Projectile | Collision")
	void OnProjectileOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                              const FHitResult& SweepResult);
	virtual void OnProjectileOverlapBegin_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                                                     bool bFromSweep, const FHitResult& SweepResult);
};
