#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/AnimationListenerProvider//EnemySpecificAnimListenerProvider.h"
#include "Interfaces/FacingDirectionProvider.h"
#include "Components/WidgetComponent.h"
#include "Templates/SubclassOf.h"
#include "EnemyCharacterBase.generated.h"

class UEnemyAISettingsDA;
class UBehaviorTree;
class UHealthComponent;
class UPaperZDAnimInstance;
class AAIController;
class UEnemyAnimInstanceBase;
class UCharacterMovementSettingsDA;
class UUserWidget;

UCLASS(Abstract)
class MY2DGAMEDESIGN_API AEnemyCharacterBase : public APaperZDCharacter,
                                               public IDamageable,
                                               public IEnemySpecificAnimListenerProvider,
                                               public IFacingDirectionProvider
{
	GENERATED_BODY()

public:
	AEnemyCharacterBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | Configuration")
	TObjectPtr<UEnemyAISettingsDA> AISettings;

	UFUNCTION(BlueprintPure, Category = "Components | Health")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI | Configuration")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
    virtual void PostInitializeComponents() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Visuals", meta=(AllowPrivateAccess="true"))
	bool bAssetFacesRightByDefault;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="Animation",
		meta=(AllowPrivateAccess = "true"))
	TWeakObjectPtr<UPaperZDAnimInstance> CachedAnimInstancePtr;

	UFUNCTION() 
	virtual void HandleDeath(AActor* Killer);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="State | Direction", meta=(AllowPrivateAccess="true"))
	bool bIsFacingRight;

	UFUNCTION()
	virtual void OnHealthChangedHandler(float CurrentHealth, float MaxHealth);

private:
	void CacheBaseAnimInstance();
	void ApplyMovementSettings();
};
