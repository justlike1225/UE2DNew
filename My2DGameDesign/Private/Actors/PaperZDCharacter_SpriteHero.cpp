#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/PlayerController.h"
#include "PaperZDAnimationComponent.h"
#include "AniInstance/HeroPaperZDAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/DashComponent.h"
#include "Components/AfterimageComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/HeroCombatComponent.h"
#include "Components/HealthComponent.h"
#include "Components/RageComponent.h"
#include "Interfaces/InputBindingComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "DataAssets/CharacterMovementSettingsDA.h"
#include "DataAssets/HeroDA/HeroRageDashSkillSettingsDA.h"
#include "DataAssets/HeroDA/HeroUpwardSweepSettingsDA.h"
#include "Engine/Engine.h"
#include "Utils/CombatGameplayStatics.h"


APaperZDCharacter_SpriteHero::APaperZDCharacter_SpriteHero()
{
	PrimaryActorTick.bCanEverTick = false;


	AfterimageComponent = CreateDefaultSubobject<UAfterimageComponent>(TEXT("AfterimageComponent"));
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	CombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("CombatComponent"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	RageComponent = CreateDefaultSubobject<URageComponent>(TEXT("RageComponent"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));


	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->RotationRate = FRotator(0.f, 0.f, 0.f);
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
		MoveComp->MaxAcceleration = 3000.0f;
		MoveComp->BrakingDecelerationWalking = 1000.0f;
	}


	SetupCamera();


	bIsWalking = false;
	bIsRunning = false;
	bIsCanJump = false;
}

void APaperZDCharacter_SpriteHero::NotifyHurtRecovery()
{
	if (bIsIncapacitated)
	{
		bIsIncapacitated = false;
	}
}

bool APaperZDCharacter_SpriteHero::IsPerformingUpwardSweep()
{
	return bIsPerformingUpwardSweep;
}

const UHeroUpwardSweepSettingsDA* APaperZDCharacter_SpriteHero::GetUpwardSweepSettings()
{
	return UpwardSweepSettings;
}




void APaperZDCharacter_SpriteHero::BeginPlay()
{
	Super::BeginPlay();


	ApplyMovementSettings();

	CacheMovementSpeeds();

	if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent())
	{
		UPaperZDAnimInstance* BaseAnimInstance = AnimComp->GetAnimInstance();
		if (BaseAnimInstance)
		{
			AnimationStateListener = TScriptInterface<ICharacterAnimationStateListener>(BaseAnimInstance);
		}
	}


	if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround()) { bIsCanJump = true; }
	else { bIsCanJump = false; }


	if (AnimationStateListener)
	{
		AnimationStateListener->
			Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
		if (CombatComponent)
		{
			AnimationStateListener->Execute_OnCombatStateChanged(AnimationStateListener.GetObject(),
			                                                     CombatComponent->GetComboCount());
		}
		if (DashComponent)
		{
			AnimationStateListener->Execute_OnDashStateChanged(AnimationStateListener.GetObject(),
			                                                   DashComponent->IsDashing());
		}
	}


	if (CombatComponent)
	{
		CombatComponent->OnGroundComboStarted.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleComboStarted);
		CombatComponent->OnGroundComboEnded.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleComboEnded);
		UE_LOG(LogTemp, Log, TEXT("SpriteHero: Bound to CombatComponent delegates."));
	}


	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleDeath);
		HealthComponent->OnHealthChanged.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleTakeHit);
		UE_LOG(LogTemp, Log, TEXT("SpriteHero: Bound to HealthComponent delegates."));
	}
	if (UCapsuleComponent* MainCapsule = GetCapsuleComponent())
	{
		MainCapsule->OnComponentBeginOverlap.AddDynamic(this, &APaperZDCharacter_SpriteHero::OnRageDashHit);
	}

	bMovementInputBlocked = false;
}


void APaperZDCharacter_SpriteHero::ApplyMovementSettings()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ApplyMovementSettings: CharacterMovementComponent is missing on %s!"),
		       *GetNameSafe(this));
		return;
	}
	if (MovementSettings)
	{
		UE_LOG(LogTemp, Log, TEXT("ApplyMovementSettings: Applying MovementSettings DA '%s' to %s"),
		       *GetNameSafe(MovementSettings), *GetNameSafe(this));
		MoveComp->MaxWalkSpeed = MovementSettings->MaxWalkSpeed;
		MoveComp->MaxAcceleration = MovementSettings->MaxAcceleration;
		MoveComp->GroundFriction = MovementSettings->GroundFriction;
		MoveComp->BrakingDecelerationWalking = MovementSettings->BrakingDecelerationWalking;
		MoveComp->JumpZVelocity = MovementSettings->JumpZVelocity;
		MoveComp->AirControl = MovementSettings->AirControl;
		MoveComp->GravityScale = MovementSettings->GravityScale;
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "ApplyMovementSettings: MovementSettings DataAsset is not assigned to %s. Using default CharacterMovementComponent values or previously set defaults."
		       ), *GetNameSafe(this));
	}
}


void APaperZDCharacter_SpriteHero::CacheMovementSpeeds()
{
	if (MovementSettings)
	{
		CachedWalkSpeed = MovementSettings->MaxWalkSpeed;
		CachedRunSpeed = MovementSettings->MaxRunSpeed;
		UE_LOG(LogTemp, Log, TEXT("CacheMovementSpeeds: Cached WalkSpeed=%.1f, RunSpeed=%.1f from DA %s for %s"),
		       CachedWalkSpeed, CachedRunSpeed, *GetNameSafe(MovementSettings), *GetNameSafe(this));
	}
	else
	{
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		if (MoveComp)
		{
			CachedWalkSpeed = MoveComp->MaxWalkSpeed;
			CachedRunSpeed = CachedWalkSpeed * 2.5f;
			UE_LOG(LogTemp, Warning,
			       TEXT(
				       "CacheMovementSpeeds: MovementSettings DA not found for %s. Using WalkSpeed=%.1f from MoveComp, estimated RunSpeed=%.1f"
			       ), *GetNameSafe(this), CachedWalkSpeed, CachedRunSpeed);
		}
		else
		{
			UE_LOG(LogTemp, Error,
			       TEXT(
				       "CacheMovementSpeeds: MovementSettings DA and MoveComp not found for %s! Using hardcoded defaults."
			       ), *GetNameSafe(this));
			CachedWalkSpeed = 200.f;
			CachedRunSpeed = 500.f;
		}
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (FMath::IsNearlyZero(MoveComp->MaxWalkSpeed))
		{
			MoveComp->MaxWalkSpeed = CachedWalkSpeed;
		}
	}
}

bool APaperZDCharacter_SpriteHero::CanExecuteRageDash() const
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!RageDashSkillSettings || !RageComponent || !MoveComp)
	{
		return false;
	}

	if (!MoveComp->IsMovingOnGround())
	{
		return false;
	}

	if (bIsRageDashing || bIsRageDashOnCooldown || bIsIncapacitated || (HealthComponent && HealthComponent->IsDead()))
	{
		return false;
	}
	if (RageComponent->GetCurrentRage() < RageDashSkillSettings->RageCost)
	{
		return false;
	}

	return true;
}

void APaperZDCharacter_SpriteHero::TryExecuteRageDash()
{
	if (CanExecuteRageDash())
	{
		ExecuteRageDash();
	}
}

void APaperZDCharacter_SpriteHero::ExecuteRageDash()
{
	if (!ensure(RageDashSkillSettings && RageComponent && GetCharacterMovement() && AnimationStateListener)) return;

	// 保留: 消耗怒气
	RageComponent->ConsumeRage(RageDashSkillSettings->RageCost);
	bMovementInputBlocked = true;
	// 保留: 设置状态和冷却
	bIsRageDashing = true;
	bIsRageDashOnCooldown = true;
	GetWorldTimerManager().SetTimer(RageDashCooldownTimer, this,
	                                &APaperZDCharacter_SpriteHero::OnRageDashCooldownFinished,
	                                RageDashSkillSettings->Cooldown, false);

	// 保留: 广播中断
	BroadcastActionInterrupt_Implementation();

	// 保留: 通知动画开始
	AnimationStateListener->Execute_OnRageDashStarted(AnimationStateListener.GetObject());


	// 保留: 获取移动组件和停止当前移动（防止蓄力时滑动）
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	// 保留: 缓存原始速度和重力
	OriginalMovementSpeed = MoveComp->MaxWalkSpeed;
	OriginalGravity = MoveComp->GravityScale;
	MoveComp->StopMovementKeepPathing();
}


void APaperZDCharacter_SpriteHero::ExecuteRageDashMovement()
{
	// 检查状态，确保是在RageDash期间被动画通知调用
	if (!bIsRageDashing || !RageDashSkillSettings || !GetCharacterMovement()) return;


	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	// 设置无重力
	MoveComp->GravityScale = 0.0f;

	// 计算方向并施加冲量
	FVector DashDirection = GetFacingDirection_Implementation().GetSafeNormal();
	LaunchCharacter(DashDirection * RageDashSkillSettings->DashSpeed, true, true);

	// 清空本次冲刺已击中列表
	HitActorsThisDash.Empty();

	// 启用碰撞检测（用于 OnRageDashHit）
	if (UCapsuleComponent* MainCapsule = GetCapsuleComponent())
	{
		MainCapsule->SetGenerateOverlapEvents(true);
	}

	// 启动冲刺移动的结束计时器
	GetWorldTimerManager().SetTimer(RageDashMovementTimer, this, &APaperZDCharacter_SpriteHero::EndRageDashMovement,
	                                RageDashSkillSettings->DashDuration, false);
}


void APaperZDCharacter_SpriteHero::EndRageDashMovement()
{
	if (!bIsRageDashing) return;


	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->Velocity = FVector::ZeroVector;
		MoveComp->GravityScale = OriginalGravity;
	}

	bMovementInputBlocked = false;
	bIsRageDashing = false;
}

void APaperZDCharacter_SpriteHero::OnRageDashCooldownFinished()
{
	bIsRageDashOnCooldown = false;
}

void APaperZDCharacter_SpriteHero::CancelRageDash()
{
	if (!bIsRageDashing) return;


	GetWorldTimerManager().ClearTimer(RageDashMovementTimer);


	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->StopMovementKeepPathing();
		MoveComp->GravityScale = OriginalGravity;
	}
	bMovementInputBlocked = false;

	bIsRageDashing = false;
}


FVector APaperZDCharacter_SpriteHero::GetFacingDirection_Implementation() const
{
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		bool bFacingRight = SpriteComponent->GetRelativeScale3D().X >= 0.0f;
		return bFacingRight ? FVector::ForwardVector : -FVector::ForwardVector;
	}
	return FVector::ForwardVector;
}


void APaperZDCharacter_SpriteHero::BroadcastActionInterrupt_Implementation()
{
	OnActionWillInterrupt.Broadcast();
}


TScriptInterface<ICharacterAnimationStateListener>
APaperZDCharacter_SpriteHero::GetAnimStateListener_Implementation() const
{
	if (!AnimationStateListener)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetAnimStateListener_Implementation: AnimationStateListener is null!"));
	}
	return AnimationStateListener;
}


void APaperZDCharacter_SpriteHero::InitializeMovementParameters()
{
	bIsWalking = false;
	bIsRunning = false;
	if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround()) { bIsCanJump = true; }
	else { bIsCanJump = false; }

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->RotationRate = FRotator(0.f, 0.f, 0.f);
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
	}
}


void APaperZDCharacter_SpriteHero::SetupCamera()
{
	if (Camera && RootComponent)
	{
		Camera->SetupAttachment(RootComponent);
		Camera->SetRelativeLocation(FVector(0.0f, 150.0f, 50.0f));
		Camera->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
		Camera->OrthoWidth = 600.0f;
	}
	else
	{
		if (!Camera) UE_LOG(LogTemp, Warning, TEXT("SetupCamera: Camera component is missing on %s."),
		                    *GetNameSafe(this));
		if (!RootComponent) UE_LOG(LogTemp, Warning, TEXT("SetupCamera: RootComponent is missing on %s."),
		                           *GetNameSafe(this));
	}
}


ETeamAttitude::Type APaperZDCharacter_SpriteHero::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
	if (OtherTeamAgent)
	{
		FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();

		if (OtherTeamId == TeamId) { return ETeamAttitude::Friendly; }


		if (OtherTeamId != TeamId) { return ETeamAttitude::Hostile; }
	}

	return ETeamAttitude::Neutral;
}


void APaperZDCharacter_SpriteHero::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (LocalPlayer)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->ClearAllMappings();
			if (PlayerMappingContext) { Subsystem->AddMappingContext(PlayerMappingContext, 0); }
			else
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("NotifyControllerChanged: PlayerMappingContext is not set on %s! Input will not work."),
				       *GetNameSafe(this));
			}
		}
	}
}


void APaperZDCharacter_SpriteHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this,
			                          &APaperZDCharacter_SpriteHero::OnJumpStarted);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this,
			                          &APaperZDCharacter_SpriteHero::OnJumpCompleted);
		}
		if (RunAction)
		{
			EnhancedInput->BindAction(RunAction, ETriggerEvent::Triggered, this,
			                          &APaperZDCharacter_SpriteHero::OnRunTriggered);
			EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this,
			                          &APaperZDCharacter_SpriteHero::OnRunCompleted);
		}
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this,
			                          &APaperZDCharacter_SpriteHero::OnMoveTriggered);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this,
			                          &APaperZDCharacter_SpriteHero::OnMoveCompleted);
		}
		if (RageDashAction)
		{
			EnhancedInput->BindAction(RageDashAction, ETriggerEvent::Started, this,
			                          &APaperZDCharacter_SpriteHero::HandleRageDashInputTriggered);
		}

		if (UpwardSweepAction)
		{
			EnhancedInput->BindAction(UpwardSweepAction, ETriggerEvent::Started, this,
			                          &APaperZDCharacter_SpriteHero::HandleUpwardSweepInputTriggered);
		}
		TArray<UActorComponent*> Components;
		GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			if (Component == HealthComponent) continue;

			IInputBindingComponent* InputBinder = Cast<IInputBindingComponent>(Component);
			if (InputBinder)
			{
				InputBinder->Execute_BindInputActions(Component, EnhancedInput);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error,
		       TEXT(
			       "SetupPlayerInputComponent: PlayerInputComponent is not an UEnhancedInputComponent on %s! Enhanced Input will not work."
		       ), *GetNameSafe(this));
	}
}


void APaperZDCharacter_SpriteHero::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bIsCanJump = true;
	if (CombatComponent) { CombatComponent->NotifyLanded(); }
}


void APaperZDCharacter_SpriteHero::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
                                                                    const FVector& PreviousFloorContactNormal,
                                                                    const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation,
	                                        TimeDelta);

	bIsCanJump = false;
	TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimStateListener_Implementation();
	if (Listener)
	{
		Listener->Execute_OnFallingRequested(Listener.GetObject());
	}
}


void APaperZDCharacter_SpriteHero::OnJumpStarted(const FInputActionValue& Value)
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	if (bIsIncapacitated)
	{
		return;
	}
	if (bMovementInputBlocked)
	{
		return; // 如果输入被阻止，则不执行跳跃
	}

	if (!bIsCanJump)
	{
		return;
	}


	bIsCanJump = false;


	OnActionWillInterrupt.Broadcast();


	Jump();


	TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimStateListener_Implementation();
	if (Listener)
	{
		Listener->Execute_OnJumpRequested(Listener.GetObject());
	}
}

void APaperZDCharacter_SpriteHero::OnJumpCompleted(const FInputActionValue& Value)
{
	StopJumping();
}


void APaperZDCharacter_SpriteHero::OnMoveTriggered(const FInputActionValue& Value)
{
	if (HealthComponent && HealthComponent->IsDead()) return;
	if (bIsIncapacitated) return;


	if (bMovementInputBlocked) { return; }
	const float MoveValue = Value.Get<float>();
	bool bWasWalking = bIsWalking;
	if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER)
	{
		bIsWalking = true;
		SetDirection(MoveValue);
		AddMovementInput(GetActorForwardVector(), MoveValue);
	}
	else { bIsWalking = false; }
	if (bIsWalking != bWasWalking && AnimationStateListener)
	{
		AnimationStateListener->
			Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
	}
}


void APaperZDCharacter_SpriteHero::OnMoveCompleted(const FInputActionValue& Value)
{
	bool bWasWalking = bIsWalking;
	bool bWasRunning = bIsRunning;
	bIsWalking = false;
	bIsRunning = false;
	if ((bWasWalking != bIsWalking || bWasRunning != bIsRunning) && AnimationStateListener)
	{
		AnimationStateListener->
			Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement()) { MoveComp->MaxWalkSpeed = CachedWalkSpeed; }
}

void APaperZDCharacter_SpriteHero::HandleRageDashInputTriggered(const FInputActionValue& Value)
{
	TryExecuteRageDash();
}

void APaperZDCharacter_SpriteHero::OnRunTriggered(const FInputActionValue& Value)
{
	if (HealthComponent && HealthComponent->IsDead()) return;
	if (bIsIncapacitated) return;
	if (bMovementInputBlocked) return;


	bool bWasRunning = bIsRunning;
	if (bIsWalking && !bIsRunning)
	{
		bIsRunning = true;
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement()) { MoveComp->MaxWalkSpeed = CachedRunSpeed; }

		TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimStateListener_Implementation();
		if (bIsRunning != bWasRunning && Listener)
		{
			Listener->Execute_OnIntentStateChanged(Listener.GetObject(), bIsWalking, bIsRunning);
		}
	}
}


void APaperZDCharacter_SpriteHero::OnRunCompleted(const FInputActionValue& Value)
{
	bool bWasRunning = bIsRunning;
	if (bIsRunning)
	{
		bIsRunning = false;

		if (bIsWalking)
		{
			if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
			{
				MoveComp->MaxWalkSpeed = CachedWalkSpeed;
			}
		}

		if (bIsRunning != bWasRunning && AnimationStateListener)
		{
			AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking,
			                                                     bIsRunning);
		}
	}
}


void APaperZDCharacter_SpriteHero::SetDirection(float Direction) const
{
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		float CurrentScaleX = SpriteComponent->GetRelativeScale3D().X;
		float TargetSign = (Direction > KINDA_SMALL_NUMBER)
			                   ? 1.0f
			                   : ((Direction < -KINDA_SMALL_NUMBER) ? -1.0f : FMath::Sign(CurrentScaleX));
		float AbsScaleX = FMath::Abs(CurrentScaleX);
		if (FMath::IsNearlyZero(AbsScaleX)) { AbsScaleX = 1.0f; }
		float TargetScaleX = AbsScaleX * TargetSign;
		if (!FMath::IsNearlyEqual(CurrentScaleX, TargetScaleX))
		{
			FVector CurrentScale = SpriteComponent->GetRelativeScale3D();
			SpriteComponent->SetRelativeScale3D(FVector(TargetScaleX, CurrentScale.Y, CurrentScale.Z));
		}
	}
	else { UE_LOG(LogTemp, Warning, TEXT("SetDirection: GetSprite() returned null for %s."), *GetNameSafe(this)); }
}


void APaperZDCharacter_SpriteHero::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CombatComponent)
	{
		if (CombatComponent->OnGroundComboStarted.IsBound())
			CombatComponent->OnGroundComboStarted.RemoveDynamic(
				this, &APaperZDCharacter_SpriteHero::HandleComboStarted);
		if (CombatComponent->OnGroundComboEnded.IsBound())
			CombatComponent->OnGroundComboEnded.RemoveDynamic(
				this, &APaperZDCharacter_SpriteHero::HandleComboEnded);
	}

	if (HealthComponent)
	{
		if (HealthComponent->OnDeath.IsBound())
			HealthComponent->OnDeath.RemoveDynamic(
				this, &APaperZDCharacter_SpriteHero::HandleDeath);
		if (HealthComponent->OnHealthChanged.IsBound())
			HealthComponent->OnHealthChanged.RemoveDynamic(
				this, &APaperZDCharacter_SpriteHero::HandleTakeHit);
	}
	Super::EndPlay(EndPlayReason);
}

void APaperZDCharacter_SpriteHero::HandleComboStarted()
{
	bMovementInputBlocked = true;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement()) { MoveComp->Velocity.X = 0.f; }
}

void APaperZDCharacter_SpriteHero::HandleComboEnded()
{
	bMovementInputBlocked = false;
}

float APaperZDCharacter_SpriteHero::ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser,
                                                               AController* InstigatorController,
                                                               const FHitResult& HitResult)
{
	if (!HealthComponent || HealthComponent->IsDead())
	{
		return 0.0f;
	}


	float ActualDamage = HealthComponent->TakeDamage(DamageAmount, DamageCauser, InstigatorController);


	if (ActualDamage > 0.f && !HealthComponent->IsDead())
	{
		bool bShouldInterrupt = true;
		if (bShouldInterrupt)
		{
			bIsIncapacitated = true;


			if (bMovementInputBlocked)
			{
				bMovementInputBlocked = false;
			}
			if (CombatComponent)
			{
				CombatComponent->HandleActionInterrupt();
			}
			if (bIsRageDashing)
			{
				CancelRageDash();
			}
			if (bIsPerformingUpwardSweep)
			{
				FinishUpwardSweep();
			}

			TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimStateListener_Implementation();
			if (Listener)
			{
				FVector HitDirection = FVector::ZeroVector;
				if (DamageCauser)
				{
					HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
				}
				else if (HitResult.IsValidBlockingHit()) { HitDirection = -HitResult.ImpactNormal; }


				Listener->Execute_OnTakeHit(Listener.GetObject(), ActualDamage, HitDirection, bShouldInterrupt);
			}
		}
	}

	return ActualDamage;
}

void APaperZDCharacter_SpriteHero::OnRageDashHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                 const FHitResult& SweepResult)
{
	if (!bIsRageDashing || !OtherActor || OtherActor == this || !RageDashSkillSettings)
	{
		return;
	}


	if (HitActorsThisDash.Contains(OtherActor))
	{
		return;
	}


	if (UCombatGameplayStatics::CanDamageActor(this, OtherActor))
	{
		if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
		{
			HitActorsThisDash.Add(OtherActor);


			float DamageToApply = RageDashSkillSettings->DamageAmount;
			AController* MyController = GetController();
			IDamageable::Execute_ApplyDamage(OtherActor, DamageToApply, this, MyController, SweepResult);
		}
	}
}


void APaperZDCharacter_SpriteHero::HandleDeath(AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("Hero %s has died! Killed by %s."), *GetNameSafe(this), *GetNameSafe(Killer));


	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		DisableInput(PC);
		UE_LOG(LogTemp, Log, TEXT("Hero input disabled."));
	}


	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}
	SetActorEnableCollision(false);


	TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimStateListener_Implementation();
	if (Listener)
	{
		Listener->Execute_OnDeathState(Listener.GetObject(), Killer);
		UE_LOG(LogTemp, Log, TEXT("HandleDeath: Notified Animation Listener OnDeathState."));
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("HandleDeath: Could not get CharacterAnimationStateListener to notify OnDeathState."));
	}
}


void APaperZDCharacter_SpriteHero::HandleTakeHit(float CurrentHealthVal, float MaxHealthVal)
{
}

void APaperZDCharacter_SpriteHero::HandleUpwardSweepInputTriggered(const FInputActionValue& Value)
{
	TryExecuteUpwardSweep();
}

bool APaperZDCharacter_SpriteHero::CanExecuteUpwardSweep() const
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!UpwardSweepSettings || !RageComponent || !MoveComp)
	{
		return false;
	}


	if (!MoveComp->IsMovingOnGround())
	{
		return false;
	}


	if (bIsPerformingUpwardSweep || bIsUpwardSweepOnCooldown || bIsRageDashing || bIsIncapacitated || (HealthComponent
		&& HealthComponent->IsDead()))
	{
		return false;
	}

	if (CombatComponent && (CombatComponent->GetComboCount() > 0 || CombatComponent->IsPerformingAirAttack()))
		return
			false;
	if (DashComponent && DashComponent->IsDashing()) return false;


	if (RageComponent->GetCurrentRage() < UpwardSweepSettings->RageCost)
	{
		return false;
	}

	return true;
}

void APaperZDCharacter_SpriteHero::TryExecuteUpwardSweep()
{
	if (CanExecuteUpwardSweep())
	{
		ExecuteUpwardSweep();
	}
}

void APaperZDCharacter_SpriteHero::ExecuteUpwardSweep()
{
	if (!ensure(UpwardSweepSettings && RageComponent && GetCharacterMovement() && AnimationStateListener)) return;


	RageComponent->ConsumeRage(UpwardSweepSettings->RageCost);


	bIsPerformingUpwardSweep = true;
	bMovementInputBlocked = true;
	bIsUpwardSweepOnCooldown = true;


	GetWorldTimerManager().SetTimer(UpwardSweepCooldownTimer, this,
	                                &APaperZDCharacter_SpriteHero::OnUpwardSweepCooldownFinished,
	                                UpwardSweepSettings->Cooldown, false);


	GetCharacterMovement()->StopMovementKeepPathing();


	BroadcastActionInterrupt_Implementation();


	AnimationStateListener->Execute_OnUpwardSweepStarted(AnimationStateListener.GetObject());
}


void APaperZDCharacter_SpriteHero::FinishUpwardSweep()
{
	if (!bIsPerformingUpwardSweep)
	{
		
		return;
	}


	bIsPerformingUpwardSweep = false;
	bMovementInputBlocked = false;

}

void APaperZDCharacter_SpriteHero::OnUpwardSweepCooldownFinished()
{
	bIsUpwardSweepOnCooldown = false;
}
