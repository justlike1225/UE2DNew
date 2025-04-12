#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyCharacterBase.h"
#include "Interfaces/MeleeShapeProvider.h"
#include "Interfaces/AI/Abilities/MeleeAbilityExecutor.h"
#include "Interfaces/AI/Abilities/TeleportAbilityExecutor.h"
#include "Interfaces/AI/Status/CombatStatusProvider.h"
#include "EvilCreature.generated.h"

class UEnemyMeleeAttackComponent;
class UTeleportComponent;
class UCapsuleComponent;
class UPrimitiveComponent;

namespace EvilCreatureAttackShapeNames
{
	const FName Melee1(TEXT("Melee1"));
	const FName Melee2(TEXT("Melee2"));
}

UCLASS()
class MY2DGAMEDESIGN_API AEvilCreature : public AEnemyCharacterBase,
                                         public IMeleeShapeProvider,
                                         public ICombatStatusProvider,
                                         public IMeleeAbilityExecutor,
                                         public ITeleportAbilityExecutor

{
	GENERATED_BODY()

public:
	AEvilCreature();
	virtual UPrimitiveComponent* GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const override;

	virtual bool CanPerformMeleeAttack_Implementation() const override;
	virtual bool CanPerformTeleport_Implementation() const override;
	virtual bool IsPerformingMeleeAttack_Implementation() const override;
	virtual bool IsPerformingTeleport_Implementation() const override;
	virtual bool ExecuteMeleeAttack_Implementation(AActor* Target) override;
	virtual bool ExecuteTeleportToLocation_Implementation(const FVector& TargetLocation) override;
	UFUNCTION(BlueprintPure, Category = "Components | Combat")
	UEnemyMeleeAttackComponent* GetMeleeAttackComponent() const { return MeleeAttackComponent; }

	UFUNCTION(BlueprintPure, Category = "Components | Ability")
	UTeleportComponent* GetTeleportComponent() const { return TeleportComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Combat",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnemyMeleeAttackComponent> MeleeAttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Ability",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTeleportComponent> TeleportComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Combat",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> MeleeHit1;


	virtual void BeginPlay() override;
};
