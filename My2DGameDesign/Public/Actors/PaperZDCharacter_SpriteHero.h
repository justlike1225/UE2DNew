#pragma once
#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "InputActionValue.h"
#include "PaperZDCharacter.h"
#include "Interfaces/ActionInterruptSource.h"
#include "Interfaces/AnimationListenerProvider/HeroAnimationStateProvider.h"
#include "Interfaces/FacingDirectionProvider.h"
#include "UObject/ScriptInterface.h"
#include "PaperZDCharacter_SpriteHero.generated.h"

class UCharacterMovementSettingsDA;
class UPaperZDAnimInstance;
class UHeroCombatComponent;
class UDashComponent;
class UAfterimageComponent;
class UCameraComponent;
class UBoxComponent;
class UCapsuleComponent;
class UInputMappingContext;
class UInputAction;
class UPaperFlipbookComponent;
class ICharacterAnimationStateListener;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionInterruptSignature);

UCLASS()
class MY2DGAMEDESIGN_API APaperZDCharacter_SpriteHero : public APaperZDCharacter, public IFacingDirectionProvider,
                                                        public IActionInterruptSource,
                                                        public IHeroAnimationStateProvider,
                                                        public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	APaperZDCharacter_SpriteHero();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration | Movement")
	TObjectPtr<UCharacterMovementSettingsDA> MovementSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FGenericTeamId TeamId = FGenericTeamId(0);

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	virtual FVector GetFacingDirection_Implementation() const override;

	UPROPERTY(BlueprintAssignable, Category = "Character|Events")
	FOnActionInterruptSignature OnActionWillInterrupt;

	UFUNCTION(BlueprintPure, Category = "Components")
	UDashComponent* GetDashComponent() const { return DashComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UAfterimageComponent* GetAfterimageComponent() const { return AfterimageComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UHeroCombatComponent* GetHeroCombatComponent() const { return CombatComponent; }

	virtual void BroadcastActionInterrupt_Implementation() override;

	virtual TScriptInterface<ICharacterAnimationStateListener> GetAnimStateListener_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsWalking() const { return bIsWalking; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsRunning() const { return bIsRunning; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAfterimageComponent> AfterimageComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDashComponent> DashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeroCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RunAction;

	float CachedWalkSpeed = 200.f;
	float CachedRunSpeed = 500.f;
	
	bool bIsCanJump = false;
	bool bIsWalking = false;
	bool bIsRunning = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess = "true"))
	TScriptInterface<ICharacterAnimationStateListener> AnimationStateListener;

	virtual void BeginPlay() override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
	                                              const FVector& PreviousFloorContactNormal,
	                                              const FVector& PreviousLocation, float TimeDelta) override;

	void OnJumpStarted(const FInputActionValue& Value);
	void OnJumpCompleted(const FInputActionValue& Value);
	void OnRunTriggered(const FInputActionValue& Value);
	void OnRunCompleted(const FInputActionValue& Value);
	void OnMoveTriggered(const FInputActionValue& Value);
	void OnMoveCompleted(const FInputActionValue& Value);

	void InitializeMovementParameters();
	void SetupCamera();
	void SetDirection(float Direction) const;

private:
	void ApplyMovementSettings(); 
	void CacheMovementSpeeds(); 
};
