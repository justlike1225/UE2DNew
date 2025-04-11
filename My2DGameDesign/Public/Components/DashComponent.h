#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/InputBindingComponent.h"
#include "DashComponent.generated.h"

class UHeroDashSettingsDA;
class ACharacter;
class UCharacterMovementComponent;
class UAfterimageComponent;
class UPaperFlipbookComponent;
struct FInputActionValue;
class UInputAction;
class UEnhancedInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashStateChangedSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashStartedSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashEndedSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MY2DGAMEDESIGN_API UDashComponent : public UActorComponent, public IInputBindingComponent
{
	GENERATED_BODY()

public:
	UDashComponent();
	UPROPERTY(BlueprintAssignable, Category = "Dash|Events")
	FOnDashStartedSignature OnDashStarted_Event;
	UPROPERTY(BlueprintAssignable, Category = "Dash|Events")
	FOnDashEndedSignature OnDashEnded_Event;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DashAction;

	virtual void BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent) override;

	UPROPERTY(BlueprintAssignable, Category = "Dash|Events")
	FOnDashStateChangedSignature OnDashStarted;
	UPROPERTY(BlueprintAssignable, Category = "Dash|Events")
	FOnDashEndedSignature OnDashEnded;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UHeroDashSettingsDA> DashSettings;

	UFUNCTION()
	void HandleDashInputTriggered(const FInputActionValue& Value);

	void PerformDash();
	UFUNCTION()
	void EndDash();
	UFUNCTION()
	void ResetDashCooldown();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintPure, Category = "Dash|State")
	bool IsDashing() const { return bIsDashing; }

	UFUNCTION(BlueprintPure, Category = "Dash|State")
	bool CanDash() const { return bCanDash && !bIsDashing; }

private:
	void ExecuteDashLogic();
	float CurrentDashSpeed = 1500.f;
	float CurrentDashDuration = 0.2f;
	float CurrentDashCooldown = 1.0f;
	FTimerHandle DashEndTimer;
	FTimerHandle DashCooldownTimer;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dash|State", meta=(AllowPrivateAccess = "true"))
	bool bIsDashing = false;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dash|State", meta=(AllowPrivateAccess = "true"))
	bool bCanDash = true;
	float DashDirectionMultiplier = 1.0f;
	UPROPERTY()
	TWeakObjectPtr<ACharacter> OwnerCharacter;
	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
	UPROPERTY()
	TWeakObjectPtr<UAfterimageComponent> OwnerAfterimageComponent;

	float OriginalGroundFriction = 8.0f;
	float OriginalMaxWalkSpeed = 600.0f;
	float OriginalGravityScale = 1.0f;
};
