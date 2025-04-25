
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
#include "PaperZDCharacter_SpriteHero.generated.h"

class URageComponent;
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
class UHeroRageDashSkillSettingsDA;
struct FHitResult; 

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
	void NotifyHurtRecovery();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FGenericTeamId TeamId = FGenericTeamId(0);
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration | Movement")
	TObjectPtr<UCharacterMovementSettingsDA> MovementSettings;
	virtual FVector GetFacingDirection_Implementation() const override;
    UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsWalking() const { return bIsWalking; }
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsRunning() const { return bIsRunning; }

	
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

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character State", meta=(AllowPrivateAccess="true"))
	bool bIsIncapacitated = false;
    
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Rage", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URageComponent> RageComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RunAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RageDashAction;
	TSet<TWeakObjectPtr<AActor>> HitActorsThisDash;
	
	UFUNCTION() 
	void OnRageDashHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 
	float CachedWalkSpeed = 200.f;
	float CachedRunSpeed = 500.f;
	bool bIsCanJump = false;
	bool bIsWalking = false;
	bool bIsRunning = false;
    bool bMovementInputBlocked = false;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character State|Skills", meta=(AllowPrivateAccess="true"))
	bool bIsRageDashing = false;
	bool bIsRageDashOnCooldown = false;
	FTimerHandle RageDashMovementTimer;
	FTimerHandle RageDashCooldownTimer;
	float OriginalMovementSpeed = 0.f; // 用于存储原始速度
	float OriginalGravity = 1.f;    // 用于存储原始重力
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess = "true"))
	TScriptInterface<ICharacterAnimationStateListener> AnimationStateListener;
	UPROPERTY(EditDefaultsOnly, Category = "Configuration | Skills")
	TObjectPtr<UHeroRageDashSkillSettingsDA> RageDashSkillSettings;
    
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
	void HandleRageDashInputTriggered(const FInputActionValue& Value);
    
	void InitializeMovementParameters();
	void SetupCamera();
	void SetDirection(float Direction) const;
    void ApplyMovementSettings();
	void CacheMovementSpeeds();
	// --- Skill Logic ---
	bool CanExecuteRageDash() const; // <--- 新增检查函数
	void TryExecuteRageDash();      // <--- 新增触发函数
	void ExecuteRageDash();         // <--- 新增核心逻辑函数
	void EndRageDashMovement();     // <--- 新增结束移动处理函数
	void OnRageDashCooldownFinished();// <--- 新增冷却结束处理函数
	void CancelRageDash();          // <--- 为后续中断处理添加
    
	UFUNCTION()
	void HandleComboStarted();
	UFUNCTION()
	void HandleComboEnded();

    
	/** 处理生命值耗尽（死亡）事件 */
	UFUNCTION() 
	void HandleDeath(AActor* Killer); 

	/** 处理受到伤害事件 (用于非动画反馈) */
	UFUNCTION() 
	void HandleTakeHit(float CurrentHealthVal, float MaxHealthVal); 
};