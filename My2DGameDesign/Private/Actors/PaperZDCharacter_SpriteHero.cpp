#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/PlayerController.h"
#include "PaperZDAnimationComponent.h"
#include "PaperZDAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/DashComponent.h"
#include "Components/AfterimageComponent.h"
#include "Components/HeroCombatComponent.h"
#include "Components/HealthComponent.h"
#include "Interfaces/InputBindingComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "DataAssets/CharacterMovementSettingsDA.h"
#include "States/HeroStateBase.h"
#include "States/IdleState.h"
#include "States/WalkingState.h"
#include "States/RunningState.h"
#include "States/JumpingState.h"
#include "States/FallingState.h"
#include "States/AttackingState.h"
#include "States/DashingState.h"
#include "States/HurtState.h"
#include "States/DeadState.h"

APaperZDCharacter_SpriteHero::APaperZDCharacter_SpriteHero()
{
	PrimaryActorTick.bCanEverTick = true;

	AfterimageComponent = CreateDefaultSubobject<UAfterimageComponent>(TEXT("AfterimageComponent"));
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	CombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("CombatComponent"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
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

	InitialStateClass = UIdleState::StaticClass();
	CurrentState = nullptr;
}

void APaperZDCharacter_SpriteHero::InitializeStateMachine()
{
	StateInstances.Empty();
	CurrentState = nullptr;

	if (!InitialStateClass)
	{
		InitialStateClass = UIdleState::StaticClass();
	}

	GetOrCreateStateInstance<UIdleState>();
	GetOrCreateStateInstance<UWalkingState>();
	GetOrCreateStateInstance<URunningState>();
	GetOrCreateStateInstance<UJumpingState>();
	GetOrCreateStateInstance<UFallingState>();
	GetOrCreateStateInstance<UHurtState>();
	GetOrCreateStateInstance<UDeadState>();

	if (InitialStateClass)
	{
		if (TObjectPtr<UHeroStateBase>* FoundState = StateInstances.Find(InitialStateClass))
		{
			CurrentState = *FoundState;
			if (CurrentState)
			{
				CurrentState->OnEnterState();
			}
		}
		else
		{
			if (TObjectPtr<UHeroStateBase>* IdleState = StateInstances.Find(UIdleState::StaticClass()))
			{
				CurrentState = *IdleState;
				if (CurrentState)
				{
					CurrentState->OnEnterState();
				}
			}
		}
	}

	if (!CurrentState)
	{
		if (TObjectPtr<UHeroStateBase>* IdleState = StateInstances.Find(UIdleState::StaticClass()))
		{
			CurrentState = *IdleState;
			if (CurrentState)
			{
				CurrentState->OnEnterState();
			}
		}
	}
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

	InitializeStateMachine();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleDeath);
		HealthComponent->OnHealthChanged.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleTakeHit);
	}
}

void APaperZDCharacter_SpriteHero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState)
	{
		CurrentState->TickState(DeltaTime);
	}
}

void APaperZDCharacter_SpriteHero::ChangeState(TSubclassOf<UHeroStateBase> NewStateClass)
{
	if (!NewStateClass)
	{
		return;
	}

	UHeroStateBase* FoundNewState = nullptr;
	if (TObjectPtr<UHeroStateBase>* FoundStatePtr = StateInstances.Find(NewStateClass))
	{
		FoundNewState = *FoundStatePtr;
	}

	if (!FoundNewState || FoundNewState == CurrentState)
	{
		return;
	}

	if (CurrentState)
	{
		CurrentState->OnExitState();
	}

	CurrentState = FoundNewState;
	CurrentState->OnEnterState();
}

void APaperZDCharacter_SpriteHero::NotifyHurtRecovery() const
{
	if (CurrentState)
	{
		CurrentState->HandleHurtRecovery();
	}
}

void APaperZDCharacter_SpriteHero::OnJumpStarted(const FInputActionValue& Value)
{
	if (CurrentState)
	{
		CurrentState->HandleJumpInputPressed();
		OnActionWillInterrupt.Broadcast();
	}
}

void APaperZDCharacter_SpriteHero::OnJumpCompleted(const FInputActionValue& Value)
{
	StopJumping();
	if (CurrentState)
	{
		CurrentState->HandleJumpInputReleased();
	}
}

void APaperZDCharacter_SpriteHero::OnRunTriggered(const FInputActionValue& Value)
{
	if (CurrentState)
	{
		CurrentState->HandleRunInputPressed();
	}
}

void APaperZDCharacter_SpriteHero::OnRunCompleted(const FInputActionValue& Value)
{
	if (CurrentState)
	{
		CurrentState->HandleRunInputReleased();
	}
}

void APaperZDCharacter_SpriteHero::OnMoveTriggered(const FInputActionValue& Value)
{
	const float MoveValue = Value.Get<float>();
	SetDirection(MoveValue);

	if (CurrentState)
	{
		CurrentState->HandleMoveInput(Value);
	}
}

void APaperZDCharacter_SpriteHero::OnMoveCompleted(const FInputActionValue& Value)
{
	if (CurrentState)
	{
		CurrentState->HandleMoveInput(FInputActionValue(0.0f));
	}
}

void APaperZDCharacter_SpriteHero::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (CombatComponent)
	{
		CombatComponent->NotifyLanded(); 
	}
	if (CurrentState)
	{
		CurrentState->HandleLanded(Hit);
	}
}

void APaperZDCharacter_SpriteHero::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
                                                                    const FVector& PreviousFloorContactNormal,
                                                                    const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta);
	if (CurrentState)
	{
		CurrentState->HandleWalkingOffLedge();
	}
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

	return ActualDamage;
}

void APaperZDCharacter_SpriteHero::HandleDeath(AActor* Killer)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		DisableInput(PC);
	}
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	if (CurrentState)
	{
		CurrentState->HandleDeath();
	}
	else
	{
		ChangeState(UDeadState::StaticClass());
	}
}

void APaperZDCharacter_SpriteHero::HandleTakeHit(float CurrentHealthVal, float MaxHealthVal)
{
	if (CurrentState && CurrentHealthVal < MaxHealthVal && !HealthComponent->IsDead())
	{
		CurrentState->HandleTakeDamage();
	}
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


TScriptInterface<ICharacterAnimationStateListener> APaperZDCharacter_SpriteHero::GetAnimStateListener_Implementation() const
{
	return AnimationStateListener;
}

void APaperZDCharacter_SpriteHero::ApplyMovementSettings()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp || !MovementSettings)
	{
		return;
	}
	MoveComp->MaxWalkSpeed = MovementSettings->MaxWalkSpeed;
	MoveComp->MaxAcceleration = MovementSettings->MaxAcceleration;
	MoveComp->GroundFriction = MovementSettings->GroundFriction;
	MoveComp->BrakingDecelerationWalking = MovementSettings->BrakingDecelerationWalking;
	MoveComp->JumpZVelocity = MovementSettings->JumpZVelocity;
	MoveComp->AirControl = MovementSettings->AirControl;
	MoveComp->GravityScale = MovementSettings->GravityScale;
}

void APaperZDCharacter_SpriteHero::CacheMovementSpeeds()
{
	if (MovementSettings)
	{
		CachedWalkSpeed = MovementSettings->MaxWalkSpeed;
		CachedRunSpeed = MovementSettings->MaxRunSpeed;
	}
	else
	{
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		if (MoveComp)
		{
			CachedWalkSpeed = MoveComp->MaxWalkSpeed;
			CachedRunSpeed = CachedWalkSpeed * 2.5f;
		}
		else
		{
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
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APaperZDCharacter_SpriteHero::OnJumpStarted);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &APaperZDCharacter_SpriteHero::OnJumpCompleted);
		}
		if (RunAction)
		{
			EnhancedInput->BindAction(RunAction, ETriggerEvent::Triggered, this, &APaperZDCharacter_SpriteHero::OnRunTriggered);
			EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &APaperZDCharacter_SpriteHero::OnRunCompleted);
		}
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APaperZDCharacter_SpriteHero::OnMoveTriggered);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &APaperZDCharacter_SpriteHero::OnMoveCompleted);
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
}

void APaperZDCharacter_SpriteHero::SetDirection(float Direction) const
{
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		float CurrentScaleX = SpriteComponent->GetRelativeScale3D().X;
		float TargetSign = 1.0f;
		if (Direction > KINDA_SMALL_NUMBER) TargetSign = 1.0f;
		else if (Direction < -KINDA_SMALL_NUMBER) TargetSign = -1.0f;
		else if (!FMath::IsNearlyZero(CurrentScaleX)) TargetSign = FMath::Sign(CurrentScaleX);

		float AbsScaleX = FMath::Abs(CurrentScaleX);
		if (FMath::IsNearlyZero(AbsScaleX)) { AbsScaleX = 1.0f; }
		float TargetScaleX = AbsScaleX * TargetSign;
		if (!FMath::IsNearlyEqual(CurrentScaleX, TargetScaleX))
		{
			FVector CurrentScale = SpriteComponent->GetRelativeScale3D();
			SpriteComponent->SetRelativeScale3D(FVector(TargetScaleX, CurrentScale.Y, CurrentScale.Z));
		}
	}
}

void APaperZDCharacter_SpriteHero::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HealthComponent)
	{
		if (HealthComponent->OnDeath.IsBound()) HealthComponent->OnDeath.RemoveDynamic(this, &APaperZDCharacter_SpriteHero::HandleDeath);
		if (HealthComponent->OnHealthChanged.IsBound()) HealthComponent->OnHealthChanged.RemoveDynamic(this, &APaperZDCharacter_SpriteHero::HandleTakeHit);
	}

	StateInstances.Empty();
	CurrentState = nullptr;

	Super::EndPlay(EndPlayReason);
}

UCharacterMovementComponent* APaperZDCharacter_SpriteHero::GetMovementComponent_Implementation() const
{
	return GetCharacterMovement();
}

UHealthComponent* APaperZDCharacter_SpriteHero::GetHealthComponent_Implementation() const
{
	return HealthComponent;
}

void APaperZDCharacter_SpriteHero::RequestStateChange_Implementation(TSubclassOf<UHeroStateBase> NewStateClass)
{
	ChangeState(NewStateClass);
}

void APaperZDCharacter_SpriteHero::PerformJump_Implementation()
{
	Jump();
}

void APaperZDCharacter_SpriteHero::PerformStopJumping_Implementation()
{
	StopJumping();
}

void APaperZDCharacter_SpriteHero::ApplyMovementInput_Implementation(const FVector& WorldDirection, float ScaleValue)
{
	AddMovementInput(WorldDirection, ScaleValue);
}

void APaperZDCharacter_SpriteHero::RequestAttack_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->HandleAttackInputTriggered(FInputActionValue());
	}
}

void APaperZDCharacter_SpriteHero::RequestDash_Implementation()
{
	if (DashComponent)
	{
		DashComponent->HandleDashInputTriggered(FInputActionValue());
	}
}


float APaperZDCharacter_SpriteHero::GetCachedWalkSpeed_Implementation() const
{
	return CachedWalkSpeed;
}

float APaperZDCharacter_SpriteHero::GetCachedRunSpeed_Implementation() const
{
	return CachedRunSpeed;
}

const AActor* APaperZDCharacter_SpriteHero::GetOwningActor_Implementation() const
{
	return  this;
}

