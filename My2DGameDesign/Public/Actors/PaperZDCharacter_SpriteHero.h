// 文件路径: My2DGameDesign/Public/Actors/PaperZDCharacter_SpriteHero.h
#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "InputActionValue.h"
#include "PaperZDCharacter.h"
#include "Interfaces/ActionInterruptSource.h"
#include "Interfaces/AnimationListenerProvider/HeroAnimationStateProvider.h"
#include "Interfaces/FacingDirectionProvider.h"
#include "UObject/ScriptInterface.h"
#include "Interfaces/Damageable.h"
#include "Templates/SubclassOf.h"
#include "PaperZDCharacter_SpriteHero.generated.h"

// --- 前向声明 ---
class UHealthComponent;
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
struct FHitResult;
class UHeroStateBase;
class UIdleState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionInterruptSignature);

UCLASS()
class MY2DGAMEDESIGN_API APaperZDCharacter_SpriteHero : public APaperZDCharacter,
                                                        public IFacingDirectionProvider,
                                                        public IActionInterruptSource,
                                                        public IHeroAnimationStateProvider,
                                                        public IGenericTeamAgentInterface,
                                                        public IDamageable
{
	GENERATED_BODY()

public:
	APaperZDCharacter_SpriteHero();

	UFUNCTION(BlueprintCallable, Category = "State Management")
	void ChangeState(TSubclassOf<UHeroStateBase> NewStateClass);

	UFUNCTION(BlueprintPure, Category = "State Management")
	UHeroStateBase* GetCurrentState() const { return CurrentState; }

	void NotifyHurtRecovery() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FGenericTeamId TeamId = FGenericTeamId(0);
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration | Movement")
	TObjectPtr<UCharacterMovementSettingsDA> MovementSettings;
	virtual FVector GetFacingDirection_Implementation() const override;

	UPROPERTY(BlueprintAssignable, Category = "Character|Events")
	FOnActionInterruptSignature OnActionWillInterrupt;
	virtual void BroadcastActionInterrupt_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "Components")
	UDashComponent* GetDashComponent() const { return DashComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UAfterimageComponent* GetAfterimageComponent() const { return AfterimageComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UHeroCombatComponent* GetHeroCombatComponent() const { return CombatComponent; }
	UFUNCTION(BlueprintPure, Category = "Components | Health")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	virtual TScriptInterface<ICharacterAnimationStateListener> GetAnimStateListener_Implementation() const override;

	virtual float ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser, AController* InstigatorController, const FHitResult& HitResult) override;

	UFUNCTION(BlueprintPure, Category = "Movement | Configuration")
	float GetCachedWalkSpeed() const { return CachedWalkSpeed; }
	UFUNCTION(BlueprintPure, Category = "Movement | Configuration")
	float GetCachedRunSpeed() const { return CachedRunSpeed; }


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAfterimageComponent> AfterimageComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDashComponent> DashComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeroCombatComponent> CombatComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;

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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess = "true"))
	TScriptInterface<ICharacterAnimationStateListener> AnimationStateListener;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State Management", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UHeroStateBase> CurrentState;

	UPROPERTY(Transient)
	TMap<TSubclassOf<UHeroStateBase>, TObjectPtr<UHeroStateBase>> StateInstances;

	UPROPERTY(EditDefaultsOnly, Category = "State Management")
	TSubclassOf<UHeroStateBase> InitialStateClass;


	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent( UInputComponent* PlayerInputComponent) override;
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

	void InitializeStateMachine();
	void SetupCamera();
	void SetDirection(float Direction) const;
	void ApplyMovementSettings();
	void CacheMovementSpeeds();

	UFUNCTION()
	void HandleDeath(AActor* Killer);
	UFUNCTION()
	void HandleTakeHit(float CurrentHealthVal, float MaxHealthVal);

	template <typename StateType>
	StateType* GetOrCreateStateInstance();

};

template <typename StateType>
StateType* APaperZDCharacter_SpriteHero::GetOrCreateStateInstance()
{
	static_assert(TIsDerivedFrom<StateType, UHeroStateBase>::IsDerived, "StateType must be derived from UHeroStateBase");

	TSubclassOf<UHeroStateBase> StateClass = StateType::StaticClass();

	if (TObjectPtr<UHeroStateBase>* FoundState = StateInstances.Find(StateClass))
	{
		return Cast<StateType>(*FoundState);
	}
	else
	{
		StateType* NewState = NewObject<StateType>(this, StateClass);
		if (NewState)
		{
			NewState->InitState(this);
			StateInstances.Add(StateClass, NewState);
			return NewState;
		}
	}
	return nullptr;
}