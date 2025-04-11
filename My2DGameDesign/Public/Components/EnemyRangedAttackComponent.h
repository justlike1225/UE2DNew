#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyRangedAttackComponent.generated.h"

class UEnemyRangedAttackSettingsDA;
class AEnemyCharacterBase;
class AActor;
class AEnemyProjectileBase;
class FTimerManager;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MY2DGAMEDESIGN_API UEnemyRangedAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyRangedAttackComponent();

	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Ranged")
	bool ExecuteAttack(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Ranged | AnimNotify")
	void HandleSpawnProjectile();

	UFUNCTION(BlueprintPure, Category = "Enemy Attack | Ranged | Status")
	bool CanAttack() const { return bCanAttack; }

	UFUNCTION(BlueprintPure, Category = "Enemy Attack | Ranged | Status")
	bool IsAttacking() const { return bIsAttacking; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack | Ranged | Configuration")
	TObjectPtr<UEnemyRangedAttackSettingsDA> AttackSettings;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Ranged | Status",
		meta=(AllowPrivateAccess="true"))
	bool bCanAttack = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Ranged | Status",
		meta=(AllowPrivateAccess="true"))
	bool bIsAttacking = false;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Enemy Attack | Ranged | Status",
		meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<AActor> CurrentTarget;

	FTimerHandle AttackCooldownTimer;

	UPROPERTY(Transient)
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;

private:
	void StartAttackCooldown();

	UFUNCTION()
	void OnAttackCooldownFinished();


	bool CalculateSpawnTransform(FVector& OutSpawnLocation, FRotator& OutSpawnRotation) const;
};
