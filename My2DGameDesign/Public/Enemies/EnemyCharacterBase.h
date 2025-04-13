#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/AnimationListenerProvider//EnemySpecificAnimListenerProvider.h"
#include "Interfaces/FacingDirectionProvider.h"
#include "EnemyCharacterBase.generated.h"

class UEnemyAISettingsDA;
class UBehaviorTree;
class UHealthComponent;
class UPaperZDAnimInstance;
class AAIController;
class UEnemyAnimInstanceBase;
class UCharacterMovementSettingsDA;
UCLASS(Abstract)
class MY2DGAMEDESIGN_API AEnemyCharacterBase : public APaperZDCharacter,
                                               public IDamageable,
                                               public IEnemySpecificAnimListenerProvider,
                                               public IFacingDirectionProvider
{
	GENERATED_BODY()

public:
	AEnemyCharacterBase();
	/** @brief 该敌人使用的 AI 配置数据资产 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | Configuration")
	TObjectPtr<UEnemyAISettingsDA> AISettings;
	UFUNCTION(BlueprintPure, Category = "Components | Health")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	/** @brief 该敌人使用的运动属性配置数据资产 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration | Movement")
	TObjectPtr<UCharacterMovementSettingsDA> MovementSettings;
	virtual float ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser,
	                                         AController* InstigatorController, const FHitResult& HitResult) override;

	virtual TScriptInterface<IEnemyMovementAnimListener> GetMovementAnimListener_Implementation() const override;
	virtual TScriptInterface<IEnemyStateAnimListener> GetStateAnimListener_Implementation() const override;
	virtual TScriptInterface<IEnemyMeleeAttackAnimListener> GetMeleeAttackAnimListener_Implementation() const override;
	virtual TScriptInterface<IEnemyRangedAttackAnimListener>
	GetRangedAttackAnimListener_Implementation() const override;
	virtual TScriptInterface<IEnemyTeleportAnimListener> GetTeleportAnimListener_Implementation() const override;

	virtual FVector GetFacingDirection_Implementation() const override;

	UPROPERTY(EditDefaultsOnly, Category="AI | Configuration")
	TObjectPtr<UBehaviorTree> BehaviorTree;
	virtual void SetFacingDirection(bool bFaceRight);

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Visuals", meta=(AllowPrivateAccess="true"))
	bool bAssetFacesRightByDefault ;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Animation",
		meta=(AllowPrivateAccess = "true"))
	TWeakObjectPtr<UPaperZDAnimInstance> CachedAnimInstancePtr;

	UFUNCTION()
	virtual void HandleDeath(AActor* Killer);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="State | Direction", meta=(AllowPrivateAccess="true"))
	bool bIsFacingRight ;

private:
	void CacheBaseAnimInstance();
	/** @brief 应用 MovementSettings 数据资产中的配置到移动组件 */ // <--- 新增辅助函数声明
	void ApplyMovementSettings();
};
