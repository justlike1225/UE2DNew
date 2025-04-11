#pragma once
#include "CoreMinimal.h"
#include "PaperZDAnimInstance.h"
#include "Interfaces/AnimationListener//EnemyMovementAnimListener.h"
#include "Interfaces/AnimationListener//EnemyStateAnimListener.h"
#include "EnemyAnimInstanceBase.generated.h"

class AEnemyCharacterBase;
class UCharacterMovementComponent;

UCLASS()
class MY2DGAMEDESIGN_API UEnemyAnimInstanceBase : public UPaperZDAnimInstance,
                                                  public IEnemyMovementAnimListener,
                                                  public IEnemyStateAnimListener
{
	GENERATED_BODY()

public:
	UEnemyAnimInstanceBase();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement",
		meta = (AllowPrivateAccess = "true"))
	float Speed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | State",
		meta = (AllowPrivateAccess = "true"))
	bool bIsHurt = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | State",
		meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

	virtual void OnInit_Implementation() override;
	virtual void OnTick_Implementation(float DeltaTime) override;

	virtual void OnMovementStateChanged_Implementation(float InSpeed, bool bInIsFalling, bool bInIsMoving) override;

	virtual void OnDeathState_Implementation(AActor* Killer) override;
	virtual void OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection,
	                                      bool bInterruptsCurrentAction) override;

	UPROPERTY(Transient, BlueprintReadOnly, Category="References", meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
};
