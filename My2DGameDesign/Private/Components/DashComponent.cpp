#include "Components/DashComponent.h"
#include "DataAssets/HeroDA/HeroDashSettingsDA.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AfterimageComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "Interfaces/ActionInterruptSource.h"
#include "Interfaces/AnimationListenerProvider/HeroAnimationStateProvider.h"
#include "Interfaces/AnimationListener//CharacterAnimationStateListener.h"
#include "Interfaces/FacingDirectionProvider.h"

UDashComponent::UDashComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bCanDash = true;
	bIsDashing = false;
	CurrentDashSpeed = 1500.f;
	CurrentDashDuration = 0.2f;
	CurrentDashCooldown = 1.0f;
	OriginalGravityScale = 1.0f;
	OriginalGroundFriction = 8.0f;
	OriginalMaxWalkSpeed = 600.0f;
}

void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter.IsValid())
	{
		OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
		OwnerAfterimageComponent = OwnerCharacter->FindComponentByClass<UAfterimageComponent>();

		if (OwnerMovementComponent.IsValid())
		{
			OriginalGroundFriction = OwnerMovementComponent->GroundFriction;
			OriginalMaxWalkSpeed = OwnerMovementComponent->MaxWalkSpeed;
			OriginalGravityScale = OwnerMovementComponent->GravityScale;
		}
	}

	if (DashSettings)
	{
		CurrentDashSpeed = DashSettings->DashSpeed;
		CurrentDashDuration = DashSettings->DashDuration;
		CurrentDashCooldown = DashSettings->DashCooldown;
	}
}

void UDashComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DashEndTimer);
		GetWorld()->GetTimerManager().ClearTimer(DashCooldownTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void UDashComponent::BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (EnhancedInputComponent && DashAction)
	{
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this,
		                                   &UDashComponent::HandleDashInputTriggered);
	}
}

void UDashComponent::HandleDashInputTriggered(const FInputActionValue& Value)
{
	ExecuteDashLogic();
}

void UDashComponent::ExecuteDashLogic()
{
	if (!CanDash())
	{
		return;
	}
	if (!OwnerCharacter.IsValid() || !OwnerMovementComponent.IsValid())
	{
		return;
	}

	PerformDash();
}

void UDashComponent::PerformDash()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerMovementComponent.IsValid() || !GetWorld())
	{
		return;
	}

	FVector DashDirection = FVector::ForwardVector;
	IFacingDirectionProvider* DirectionProvider = Cast<IFacingDirectionProvider>(OwnerActor);
	if (DirectionProvider)
	{
		DashDirection = IFacingDirectionProvider::Execute_GetFacingDirection(OwnerActor);
	}
	else
	{
		DashDirection = OwnerActor->GetActorForwardVector();
	}

	IActionInterruptSource* InterruptSource = Cast<IActionInterruptSource>(OwnerActor);
	if (InterruptSource)
	{
		IActionInterruptSource::Execute_BroadcastActionInterrupt(OwnerActor);
	}

	TScriptInterface<ICharacterAnimationStateListener> Listener = nullptr;
	IHeroAnimationStateProvider* AnimProvider = Cast<IHeroAnimationStateProvider>(OwnerActor);
	if (AnimProvider)
	{
		Listener = IHeroAnimationStateProvider::Execute_GetAnimStateListener(OwnerActor);
		if (!Listener.GetInterface())
		{
			Listener = nullptr;
		}
	}

	bool bWasDashing = bIsDashing;
	bIsDashing = true;
	bCanDash = false;

	if (Listener && bIsDashing != bWasDashing)
	{
		Listener->Execute_OnDashStateChanged(Listener.GetObject(), true);
	}

	OriginalGroundFriction = OwnerMovementComponent->GroundFriction;
	OriginalMaxWalkSpeed = OwnerMovementComponent->MaxWalkSpeed;
	OriginalGravityScale = OwnerMovementComponent->GravityScale;
	FVector TargetDashVelocity = DashDirection.GetSafeNormal() * CurrentDashSpeed;
	OwnerMovementComponent->GravityScale = 0.0f;
	OwnerMovementComponent->GroundFriction = 0.0f;
	OwnerMovementComponent->StopMovementKeepPathing();
	OwnerMovementComponent->Velocity = TargetDashVelocity;

	OnDashStarted_Event.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(DashEndTimer, this, &UDashComponent::EndDash, CurrentDashDuration, false);
	GetWorld()->GetTimerManager().SetTimer(DashCooldownTimer, this, &UDashComponent::ResetDashCooldown,
	                                       CurrentDashCooldown, false);
}

void UDashComponent::EndDash()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerMovementComponent.IsValid() || !GetWorld() || !bIsDashing)
	{
		return;
	}

	TScriptInterface<ICharacterAnimationStateListener> Listener = nullptr;
	IHeroAnimationStateProvider* AnimProvider = Cast<IHeroAnimationStateProvider>(OwnerActor);
	if (AnimProvider)
	{
		Listener = IHeroAnimationStateProvider::Execute_GetAnimStateListener(OwnerActor);
		if (!Listener.GetInterface()) { Listener = nullptr; }
	}

	bool bWasDashing = bIsDashing;
	bIsDashing = false;

	if (Listener && bIsDashing != bWasDashing)
	{
		Listener->Execute_OnDashStateChanged(Listener.GetObject(), false);
	}

	OnDashEnded_Event.Broadcast();

	OwnerMovementComponent->GravityScale = OriginalGravityScale;
	OwnerMovementComponent->GroundFriction = OriginalGroundFriction;
	OwnerMovementComponent->MaxWalkSpeed = OriginalMaxWalkSpeed;
}

void UDashComponent::ResetDashCooldown()
{
	bCanDash = true;
}
