#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "Containers/Set.h"
#include "EnemyMeleeAttackComponent.generated.h"

enum class  EEnemyMeleeAttackType : uint8;
class UEnemyMeleeAttackSettingsDA;
class AEnemyCharacterBase;
class AActor;
class IEnemyMeleeAttackAnimListener;
template <class InterfaceType>
class TScriptInterface;
class UPrimitiveComponent;
struct FHitResult;
class UDamageable;
class AController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MY2DGAMEDESIGN_API UEnemyMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyMeleeAttackComponent();


	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee") 
	bool ExecuteAttack(EEnemyMeleeAttackType AttackType, AActor* Target = nullptr); 
	UFUNCTION(BlueprintPure, Category = "Enemy Attack | Melee | Status")
	bool CanAttack() const { return bCanAttack; }

	UFUNCTION(BlueprintPure, Category = "Enemy Attack | Melee | Status")
	bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee | AnimNotify")
	void ActivateMeleeCollision(FName ShapeIdentifier, float Duration = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Enemy Attack | Melee | AnimNotify")
	void DeactivateMeleeCollision(FName ShapeIdentifier);


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Configuration")
	TObjectPtr<UEnemyMeleeAttackSettingsDA> AttackSettings;

	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> HitActorsThisSwing;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Status",
		meta=(AllowPrivateAccess="true"))
	bool bCanAttack = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy Attack | Melee | Status",
		meta=(AllowPrivateAccess="true"))
	bool bIsAttacking = false;

	FTimerHandle AttackCooldownTimer;
	FTimerHandle ActiveCollisionTimerHandle;
	FName ActiveCollisionShapeName;


	UPROPERTY(Transient)
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;

	UFUNCTION()
	virtual void HandleAttackOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	void StartAttackCooldown();

	UFUNCTION()
	void OnAttackCooldownFinished();

	TScriptInterface<IEnemyMeleeAttackAnimListener> GetAnimListener() const;

	void BeginAttackSwing();
};
