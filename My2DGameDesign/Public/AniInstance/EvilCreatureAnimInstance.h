#pragma once
#include "CoreMinimal.h"
#include "AniInstance/EnemyAnimInstanceBase.h"
#include "Interfaces/AnimationListener/EnemyMeleeAttackAnimListener.h"
#include "Interfaces/AnimationListener/EnemyTeleportAnimListener.h"
#include "Interfaces/AnimationListener/MeleeStateResetListener.h"
#include "EvilCreatureAnimInstance.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UEvilCreatureAnimInstance : public UEnemyAnimInstanceBase,
                                                     public IEnemyMeleeAttackAnimListener,
                                                     public IEnemyTeleportAnimListener,
                                                     public IMeleeStateResetListener
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Combat",
		meta = (AllowPrivateAccess = "true"))
	bool bIsAttackingMelee = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Combat",
		meta = (AllowPrivateAccess = "true"))
	int32 MeleeAttackIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State | Ability",
		meta = (AllowPrivateAccess = "true"))
	bool bIsTeleporting = false;

	virtual void OnMeleeAttackStarted_Implementation(AActor* Target, int32 AttackIndex ) override;

public:
	virtual void HandleMeleeAttackEnd_Implementation() override;
	virtual void OnTeleportStateChanged_Implementation(bool bNewIsTeleporting) override;
};
