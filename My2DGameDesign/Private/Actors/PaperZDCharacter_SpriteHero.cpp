#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/PlayerController.h"
#include "PaperZDAnimInstance.h"
#include "PaperZDAnimationComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/DashComponent.h"
#include "Components/AfterimageComponent.h"
#include "Components/HeroCombatComponent.h"
#include "Interfaces/InputBindingComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "InputMappingContext.h"

APaperZDCharacter_SpriteHero::APaperZDCharacter_SpriteHero()
{
	PrimaryActorTick.bCanEverTick = false;

	AfterimageComponent = CreateDefaultSubobject<UAfterimageComponent>(TEXT("AfterimageComponent"));
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	CombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("CombatComponent"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	InitializeMovementParameters();
	SetupCamera();
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
	return AnimationStateListener;
}

void APaperZDCharacter_SpriteHero::InitializeMovementParameters()
{
	bIsWalking = false;
	bIsRunning = false;
	bIsCanJump = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->RotationRate = FRotator(0.f, 0.f, 0.f);
		MoveComp->MaxAcceleration = 3000.0f;
		MoveComp->BrakingDecelerationWalking = 1000.0f;

		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
	}
}

void APaperZDCharacter_SpriteHero::SetupCamera()
{
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(0.0f, 150.0f, 50.0f));
	Camera->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
	Camera->OrthoWidth = 600.0f;
}

ETeamAttitude::Type APaperZDCharacter_SpriteHero::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
	if (OtherTeamAgent)
	{
		FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();
		if (OtherTeamId == TeamId)
		{
			return ETeamAttitude::Friendly;
		}
		return ETeamAttitude::Hostile;
	}
	return ETeamAttitude::Neutral;
}

void APaperZDCharacter_SpriteHero::BeginPlay()
{
	Super::BeginPlay();

	if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent())
	{
		UPaperZDAnimInstance* BaseAnimInstance = AnimComp->GetAnimInstance();
		if (BaseAnimInstance)
		{
			AnimationStateListener = TScriptInterface<ICharacterAnimationStateListener>(BaseAnimInstance);
		}
	}

	if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround())
	{
		bIsCanJump = true;
	}
	else
	{
		bIsCanJump = false;
	}

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
			if (PlayerMappingContext)
			{
				Subsystem->AddMappingContext(PlayerMappingContext, 0);
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

		TArray<UActorComponent*> Components;
		GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			IInputBindingComponent* InputBinder = Cast<IInputBindingComponent>(Component);
			if (InputBinder)
			{
				InputBinder->Execute_BindInputActions(Component, EnhancedInput);
			}
		}
	}
}

void APaperZDCharacter_SpriteHero::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bIsCanJump = true;
	if (CombatComponent)
	{
		CombatComponent->NotifyLanded();
	}
}

void APaperZDCharacter_SpriteHero::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
                                                                    const FVector& PreviousFloorContactNormal,
                                                                    const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation,
	                                        TimeDelta);
	bIsCanJump = false;
}

void APaperZDCharacter_SpriteHero::OnJumpStarted(const FInputActionValue& Value)
{
	if (!bIsCanJump) { return; }
	bIsCanJump = false;
	OnActionWillInterrupt.Broadcast();
	Jump();
	if (AnimationStateListener) { AnimationStateListener->Execute_OnJumpRequested(AnimationStateListener.GetObject()); }
}

void APaperZDCharacter_SpriteHero::OnJumpCompleted(const FInputActionValue& Value)
{
	StopJumping();
}

void APaperZDCharacter_SpriteHero::OnMoveTriggered(const FInputActionValue& Value)
{
	const float MoveValue = Value.Get<float>();
	bool bWasWalking = bIsWalking;

	if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER)
	{
		bIsWalking = true;
		SetDirection(MoveValue);
		AddMovementInput(GetActorForwardVector(), MoveValue);
	}
	else
	{
		bIsWalking = false;
	}

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

	if (GetCharacterMovement()) { GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; }
}

void APaperZDCharacter_SpriteHero::OnRunTriggered(const FInputActionValue& Value)
{
	bool bWasRunning = bIsRunning;
	if (bIsWalking && !bIsRunning)
	{
		bIsRunning = true;
		if (GetCharacterMovement()) { GetCharacterMovement()->MaxWalkSpeed = RunSpeed; }

		if (bIsRunning != bWasRunning && AnimationStateListener)
		{
			AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking,
			                                                     bIsRunning);
		}
	}
}

void APaperZDCharacter_SpriteHero::OnRunCompleted(const FInputActionValue& Value)
{
	bool bWasRunning = bIsRunning;
	if (bIsRunning)
	{
		bIsRunning = false;
		if (bIsWalking && GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
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
		float TargetScaleX = (Direction > KINDA_SMALL_NUMBER)
			                     ? 1.0f
			                     : ((Direction < -KINDA_SMALL_NUMBER) ? -1.0f : CurrentScaleX);

		if (!FMath::IsNearlyEqual(CurrentScaleX, TargetScaleX))
		{
			FVector CurrentScale = SpriteComponent->GetRelativeScale3D();
			SpriteComponent->SetRelativeScale3D(FVector(TargetScaleX, CurrentScale.Y, CurrentScale.Z));
		}
	}
}
