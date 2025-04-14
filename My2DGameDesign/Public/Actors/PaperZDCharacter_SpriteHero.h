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
#include "Interfaces/Damageable.h" // <--- 包含 IDamageable 接口头文件
#include "PaperZDCharacter_SpriteHero.generated.h"

class UHealthComponent; // <--- 前向声明 UHealthComponent
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
struct FHitResult; // <--- 前向声明 FHitResult

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionInterruptSignature);

UCLASS()
class MY2DGAMEDESIGN_API APaperZDCharacter_SpriteHero : public APaperZDCharacter,
                                                        public IFacingDirectionProvider,
                                                        public IActionInterruptSource,
                                                        public IHeroAnimationStateProvider,
                                                        public IGenericTeamAgentInterface,
                                                        public IDamageable // <--- 添加 IDamageable 到继承列表
{
	GENERATED_BODY()

public:
	APaperZDCharacter_SpriteHero();

	// --- Team ID and Attitude ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FGenericTeamId TeamId = FGenericTeamId(0);
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

    // --- Movement and Direction ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration | Movement")
	TObjectPtr<UCharacterMovementSettingsDA> MovementSettings;
	virtual FVector GetFacingDirection_Implementation() const override;
    UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsWalking() const { return bIsWalking; }
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsRunning() const { return bIsRunning; }

	// --- Action Interrupt ---
	UPROPERTY(BlueprintAssignable, Category = "Character|Events")
	FOnActionInterruptSignature OnActionWillInterrupt;
	virtual void BroadcastActionInterrupt_Implementation() override;

	// --- Component Getters ---
	UFUNCTION(BlueprintPure, Category = "Components")
	UDashComponent* GetDashComponent() const { return DashComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UAfterimageComponent* GetAfterimageComponent() const { return AfterimageComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UHeroCombatComponent* GetHeroCombatComponent() const { return CombatComponent; }
	UFUNCTION(BlueprintPure, Category = "Components | Health")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; } // <--- Getter for HealthComponent

	// --- Animation State Provider ---
	virtual TScriptInterface<ICharacterAnimationStateListener> GetAnimStateListener_Implementation() const override;

	// --- IDamageable Interface Implementation ---
	virtual float ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser, AController* InstigatorController, const FHitResult& HitResult) override; // <--- IDamageable implementation declaration

protected:
    // --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAfterimageComponent> AfterimageComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDashComponent> DashComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeroCombatComponent> CombatComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent; // <--- HealthComponent member declaration

	// --- Input Configuration ---
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RunAction;

    // --- Movement State ---
	float CachedWalkSpeed = 200.f;
	float CachedRunSpeed = 500.f;
	bool bIsCanJump = false;
	bool bIsWalking = false;
	bool bIsRunning = false;
    bool bMovementInputBlocked = false;

	// --- Animation State Listener Cache ---
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess = "true"))
	TScriptInterface<ICharacterAnimationStateListener> AnimationStateListener;

    // --- Lifecycle & Input Handling Functions ---
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
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

    // --- Initialization & Setup ---
	void InitializeMovementParameters();
	void SetupCamera();
	void SetDirection(float Direction) const;
    void ApplyMovementSettings();
	void CacheMovementSpeeds();

    // --- Combat Delegate Handlers ---
	UFUNCTION()
	void HandleComboStarted();
	UFUNCTION()
	void HandleComboEnded();

    // --- Health Event Handlers ---
	/** 处理生命值耗尽（死亡）事件 */
	UFUNCTION() // UFUNCTION() is required for delegate binding
	void HandleDeath(AActor* Killer); // <--- Declaration for death handler

	/** 处理受到伤害事件 (用于非动画反馈) */
	UFUNCTION() // UFUNCTION() is required for delegate binding
	void HandleTakeHit(float CurrentHealthVal, float MaxHealthVal); // <--- Declaration for hit handler
};