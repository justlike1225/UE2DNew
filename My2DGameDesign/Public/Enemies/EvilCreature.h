#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyCharacterBase.h"
#include "Interfaces/MeleeShapeProvider.h"
#include "Interfaces/AI/Abilities/MeleeAbilityExecutor.h"
#include "Interfaces/AI/Abilities/TeleportAbilityExecutor.h"
#include "Interfaces/AI/Status/CombatStatusProvider.h"
#include "Interfaces/AnimationEvents/EnemyAnimationEventHandler.h"
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
                                         public ITeleportAbilityExecutor,
                                         public IEnemyAnimationEventHandler
{
	GENERATED_BODY()

public:
	AEvilCreature();
	virtual void PostInitializeComponents() override;
	virtual UPrimitiveComponent* GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const override;

	virtual bool CanPerformMeleeAttack_Implementation() const override;
	virtual bool CanPerformTeleport_Implementation() const override;
	virtual bool IsPerformingMeleeAttack_Implementation() const override;
	virtual bool IsPerformingTeleport_Implementation() const override;
	virtual bool ExecuteMeleeAttack_Implementation(EEnemyMeleeAttackType AttackType, AActor* Target) override;

	virtual bool ExecuteTeleportToLocation_Implementation(const FVector& TargetLocation) override;
	UFUNCTION(BlueprintPure, Category = "Components | Combat")
	UEnemyMeleeAttackComponent* GetMeleeAttackComponent() const { return MeleeAttackComponent; }

	UFUNCTION(BlueprintPure, Category = "Components | Ability")
	UTeleportComponent* GetTeleportComponent() const { return TeleportComponent; }

	
	/** 处理来自动画通知的“激活近战碰撞体”事件 */
	virtual void HandleAnim_ActivateMeleeCollision_Implementation(FName ShapeIdentifier, float Duration) override;
	/** 处理来自动画通知的“完成传送状态”事件 */
	virtual void HandleAnim_FinishTeleportState_Implementation() override;
	/** 处理来自动画通知的“重置近战攻击状态”事件 */
	virtual void HandleAnim_ResetMeleeState_Implementation() override;
	// --- 接口实现结束 ---
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
