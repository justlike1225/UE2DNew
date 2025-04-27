#include "Components/Skills/RageDashComponent.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/RageComponent.h"
#include "DataAssets/HeroDA/HeroRageDashSkillSettingsDA.h"
#include "EnhancedInputComponent.h"
#include "TimerManager.h"
#include "Components/HealthComponent.h"
#include "Engine/World.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "Interfaces/FacingDirectionProvider.h" 
#include "Interfaces/Damageable.h"           
#include "Utils/CombatGameplayStatics.h"     
URageDashComponent::URageDashComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}
void URageDashComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerHero = Cast<APaperZDCharacter_SpriteHero>(GetOwner());
	if (OwnerHero.IsValid())
	{
		OwnerMovementComponent = OwnerHero->GetCharacterMovement();
		OwnerRageComponent = OwnerHero->GetRageComponent();
		OwnerCapsuleComponent = OwnerHero->GetCapsuleComponent();
		if (IHeroAnimationStateProvider* Provider = Cast<IHeroAnimationStateProvider>(OwnerHero.Get()))
		{
			OwnerAnimListenerCached = Provider->Execute_GetAnimStateListener(OwnerHero.Get());
		}
		OwnerHero->OnActionWillInterrupt.AddDynamic(this, &URageDashComponent::CancelRageDash);
	}
	else
	{
		UE_LOG(LogTemp, Error,
		       TEXT("RageDashComponent is attached to a non-APaperZDCharacter_SpriteHero actor or Owner is invalid!"));
		SetActive(false);
		SetComponentTickEnabled(false);
		return;
	}
	if (!OwnerMovementComponent.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("RageDashComponent on %s requires a UCharacterMovementComponent!"),
		       *GetNameSafe(GetOwner()));
		SetActive(false); 
	}
	if (!OwnerRageComponent.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("RageDashComponent on %s requires a URageComponent!"), *GetNameSafe(GetOwner()));
		SetActive(false); 
	}
	if (!RageDashSkillSettings)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("RageDashComponent on %s is missing RageDashSkillSettingsDA! Rage Dash will likely fail."),
		       *GetNameSafe(GetOwner()));
	}
	if (!OwnerCapsuleComponent.IsValid())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "RageDashComponent on %s could not find Owner's CapsuleComponent. RageDashHit might not work if relying on Hero's overlap."
		       ), *GetNameSafe(GetOwner()));
	}
	if (!OwnerAnimListenerCached)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "RageDashComponent on %s could not find Owner's AnimationStateListener. Animations might not trigger correctly."
		       ), *GetNameSafe(GetOwner()));
	}
}
void URageDashComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RageDashMovementTimer);
		GetWorld()->GetTimerManager().ClearTimer(RageDashCooldownTimer);
	}
	if (OwnerHero.IsValid())
	{
		if (OwnerHero->OnActionWillInterrupt.IsBound())
		{
			OwnerHero->OnActionWillInterrupt.RemoveDynamic(this, &URageDashComponent::CancelRageDash);
		}
	}
	Super::EndPlay(EndPlayReason);
}
void URageDashComponent::BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (EnhancedInputComponent && RageDashAction)
	{
		EnhancedInputComponent->BindAction(RageDashAction, ETriggerEvent::Started, this,
		                                   &URageDashComponent::HandleRageDashInputTriggered);
	}
	else
	{
		if (!RageDashAction) UE_LOG(LogTemp, Warning,
		                            TEXT("RageDashComponent: RageDashAction is not set in configuration!"));
	}
}
void URageDashComponent::HandleRageDashInputTriggered(const FInputActionValue& Value)
{
	TryExecuteRageDash();
}
bool URageDashComponent::CanRageDash() const
{
	if (!RageDashSkillSettings || !OwnerRageComponent.IsValid() || !OwnerMovementComponent.IsValid() || !OwnerHero.
		IsValid())
	{
		return false;
	}
	if (!OwnerMovementComponent->IsMovingOnGround())
	{
		return false;
	}
	UHealthComponent* HealthComp = OwnerHero->GetHealthComponent(); 
	if (bIsRageDashing || bIsRageDashOnCooldown || OwnerHero->IsMovementInputBlocked() || (HealthComp && HealthComp->
		IsDead()))
	{
		return false;
	}
	if (OwnerRageComponent->GetCurrentRage() < RageDashSkillSettings->RageCost)
	{
		return false;
	}
	return true; 
}
void URageDashComponent::TryExecuteRageDash()
{
	if (CanRageDash())
	{
		ExecuteRageDash();
	}
}
void URageDashComponent::ExecuteRageDash()
{
	if (!ensure(
		RageDashSkillSettings && OwnerRageComponent.IsValid() && OwnerMovementComponent.IsValid() && OwnerHero.IsValid()
		))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			TEXT("RageDashComponent: ExecuteRageDash failed due to missing dependencies!"));
		return;
	}
	OwnerRageComponent->ConsumeRage(RageDashSkillSettings->RageCost);
	bIsRageDashing = true;
	bIsRageDashOnCooldown = true;
	OwnerHero->SetMovementInputBlocked(true);
	GetWorldTimerManager().SetTimer(RageDashCooldownTimer, this,
	                                &URageDashComponent::OnRageDashCooldownFinished,
	                                RageDashSkillSettings->Cooldown, false);

	if (auto Listener = GetAnimListener())
	{
		Listener->Execute_OnRageDashStarted(Listener.GetObject());
	}
	OriginalMovementSpeed = OwnerMovementComponent->MaxWalkSpeed;
	OriginalGravity = OwnerMovementComponent->GravityScale;
	OwnerMovementComponent->StopMovementKeepPathing(); 
}
void URageDashComponent::ExecuteRageDashMovement()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		TEXT("RageDashComponent: ExecuteRageDashMovement called!"));
	if (!bIsRageDashing || !RageDashSkillSettings || !OwnerMovementComponent.IsValid() || !OwnerHero.IsValid()) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, 
			TEXT("RageDashComponent: ExecuteRageDashMovement failed due to missing dependencies!"));
		return;
	}
	OwnerMovementComponent->GravityScale = 0.0f;
	FVector DashDirection = FVector::ForwardVector; 
	if (IFacingDirectionProvider* FacingProvider = Cast<IFacingDirectionProvider>(OwnerHero.Get()))
	{
		DashDirection = FacingProvider->Execute_GetFacingDirection(OwnerHero.Get());
	}
	else 
	{
		DashDirection = OwnerHero->GetActorForwardVector();
	}
	OwnerHero->LaunchCharacter(DashDirection.GetSafeNormal() * RageDashSkillSettings->DashSpeed, true, true);
	HitActorsThisDash.Empty();
	GetWorldTimerManager().SetTimer(RageDashMovementTimer, this, &URageDashComponent::EndRageDashMovement,
	                                RageDashSkillSettings->DashDuration, false);
}
void URageDashComponent::EndRageDashMovement()
{
	if (!bIsRageDashing || !OwnerMovementComponent.IsValid() || !OwnerHero.IsValid()) return;
	OwnerMovementComponent->Velocity = FVector::ZeroVector; 
	OwnerMovementComponent->GravityScale = OriginalGravity; 
	OwnerHero->SetMovementInputBlocked(false); 
	bIsRageDashing = false;
	TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
	if (Listener)
	{
		Listener->Execute_OnRageDashEnded(Listener.GetObject());
	}
}
void URageDashComponent::OnRageDashCooldownFinished()
{
	bIsRageDashOnCooldown = false;
}
void URageDashComponent::CancelRageDash()
{
	if (!bIsRageDashing) return;
	GetWorldTimerManager().ClearTimer(RageDashMovementTimer);
	if (OwnerMovementComponent.IsValid())
	{
		OwnerMovementComponent->StopMovementKeepPathing(); 
		OwnerMovementComponent->GravityScale = OriginalGravity; 
	}
	if (OwnerHero.IsValid()) OwnerHero->SetMovementInputBlocked(false);
	bIsRageDashing = false;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		TEXT("RageDashComponent: Rage Dash cancelled!"));

}
void URageDashComponent::HandleRageDashHit(AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                           const FHitResult& SweepResult)
{
	if (!bIsRageDashing || !OtherActor || !OwnerHero.IsValid() || OtherActor == OwnerHero.Get() || !
		RageDashSkillSettings)
	{
		return;
	}
	if (HitActorsThisDash.Contains(OtherActor))
	{
		return;
	}
	if (UCombatGameplayStatics::CanDamageActor(OwnerHero.Get(), OtherActor))
	{
		if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
		{
			HitActorsThisDash.Add(OtherActor);
			float DamageToApply = RageDashSkillSettings->DamageAmount;
			AController* MyController = OwnerHero->GetController();
			IDamageable::Execute_ApplyDamage(OtherActor, DamageToApply, OwnerHero.Get(), MyController, SweepResult);
		}
	}
}
TScriptInterface<ICharacterAnimationStateListener> URageDashComponent::GetAnimListener() const
{
	if (OwnerAnimListenerCached) return OwnerAnimListenerCached;
	if (OwnerHero.IsValid())
	{
		if (IHeroAnimationStateProvider* Provider = Cast<IHeroAnimationStateProvider>(OwnerHero.Get()))
		{
			return Provider->Execute_GetAnimStateListener(OwnerHero.Get());
		}
	}
	return nullptr;
}
FTimerManager& URageDashComponent::GetWorldTimerManager() const
{
	return GetWorld()->GetTimerManager();
}