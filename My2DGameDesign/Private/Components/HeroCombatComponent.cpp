#include "Components/HeroCombatComponent.h"
#include "EnhancedInputComponent.h"
#include "PaperZDCharacter.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "PaperFlipbookComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "DataAssets/HeroDA/HeroCombatSettingsDA.h"
#include "GameFramework/DamageType.h"
#include "TimerManager.h"
#include "Actors/SwordBeamProjectile.h"
#include "Components/RageComponent.h"
#include "DataAssets/HeroDA/HeroUpwardSweepSettingsDA.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/Damageable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utils/CombatGameplayStatics.h"
UHeroCombatComponent::UHeroCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ComboCount = 0;
	bCanCombo = true;
	bIsPerformingAirAttack = false;
	bCanAirAttack = true;
	bWantsInitializeComponent = true;
	AttackHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitbox"));
	if (AttackHitBox)
	{
		AttackHitBox->ComponentTags.Add(AttackShapeNames::AttackHitBox);
	}
	AttackHitCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("AttackHitCapsule"));
	if (AttackHitCapsule)
	{
		AttackHitCapsule->ComponentTags.Add(AttackShapeNames::AttackHitCapsule);
	}
	ThrustAttackCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ThrustAttackCapsule"));
	if (ThrustAttackCapsule)
	{
		ThrustAttackCapsule->ComponentTags.Add(AttackShapeNames::ThrustAttackCapsule);
	}
}
void UHeroCombatComponent::InitializeComponent()
{
	Super::InitializeComponent();
	OwnerCharacter = Cast<APaperZDCharacter>(GetOwner());
	if (OwnerCharacter.IsValid())
	{
		OwnerSpriteComponent = OwnerCharacter->GetSprite();
		if (APaperZDCharacter_SpriteHero* OwnerHero = Cast<APaperZDCharacter_SpriteHero>(OwnerCharacter.Get()))
		{
			OwnerHero->OnActionWillInterrupt.AddDynamic(this, &UHeroCombatComponent::HandleActionInterrupt);
		}
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
		if (AttackHitBox)
		{
			AttackHitBox->AttachToComponent(OwnerSpriteComponent.Get(), AttachmentRules);
			AttackHitBox->SetRelativeLocation(FVector(22.0f, 0.0f, 0.0f));
			AttackHitBox->SetBoxExtent(FVector(15.0f, 20.0f, 18.0f));
			ConfigureAttackCollisionComponent(AttackHitBox);
			AttackHitBox->OnComponentBeginOverlap.AddDynamic(this, &UHeroCombatComponent::OnAttackHit);
		}
		if (AttackHitCapsule)
		{
			AttackHitCapsule->AttachToComponent(OwnerSpriteComponent.Get(), AttachmentRules);
			AttackHitCapsule->SetRelativeLocation(FVector(11.23f, 0.0f, 9.02f));
			AttackHitCapsule->SetRelativeRotation(FRotator(46.0f, 0.0f, 0.0f));
			AttackHitCapsule->SetCapsuleSize(12.0f, 27.0f);
			ConfigureAttackCollisionComponent(AttackHitCapsule);
			AttackHitCapsule->OnComponentBeginOverlap.AddDynamic(this, &UHeroCombatComponent::OnAttackHit);
		}
		if (ThrustAttackCapsule)
		{
			ThrustAttackCapsule->AttachToComponent(OwnerSpriteComponent.Get(), AttachmentRules);
			ThrustAttackCapsule->SetRelativeLocation(FVector(48.0f, 0.0f, 0.0f));
			ThrustAttackCapsule->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
			ThrustAttackCapsule->SetCapsuleSize(7.0f, 27.0f);
			ConfigureAttackCollisionComponent(ThrustAttackCapsule);
			ThrustAttackCapsule->OnComponentBeginOverlap.AddDynamic(this, &UHeroCombatComponent::OnAttackHit);
		}
	}
}
void UHeroCombatComponent::ConfigureAttackCollisionComponent(UPrimitiveComponent* CollisionComp, FName ProfileName)
{
	if (CollisionComp)
	{
		CollisionComp->SetCollisionProfileName(ProfileName);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		if (OwnerCharacter.IsValid())
		{
			CollisionComp->IgnoreActorWhenMoving(OwnerCharacter.Get(), true);
		}
		CollisionComp->CanCharacterStepUpOn = ECB_No;
		CollisionComp->SetGenerateOverlapEvents(true);
	}
}
void UHeroCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AttackHitBox)
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (AttackHitCapsule)
	{
		AttackHitCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (ThrustAttackCapsule)
	{
		ThrustAttackCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (CombatSettings)
	{
		CurrentComboResetDelay = CombatSettings->ComboResetDelayAfterWindowClose;
		CurrentGroundAttackCooldownDuration = CombatSettings->GroundAttackCooldownDuration;
		CurrentGroundBaseAttackDamage = CombatSettings->GroundBaseAttackDamage;
		CurrentMaxGroundComboCount = CombatSettings->MaxGroundComboCount;
		CurrentAirAttackMeleeDamage = CombatSettings->AirAttackMeleeDamage;
		CurrentAirAttackCooldownDuration = CombatSettings->AirAttackCooldownDuration;
		CurrentSwordBeamClass = CombatSettings->SwordBeamClass;
		CurrentSwordBeamSpawnOffset = CombatSettings->SwordBeamSpawnOffset;
		CurrentSwordBeamInitialSpeed = CombatSettings->SwordBeamInitialSpeed;
		CurrentSwordBeamDamage = CombatSettings->SwordBeamDamage;
		CurrentSwordBeamLifeSpan = CombatSettings->SwordBeamLifeSpan;
	}
	ComboCount = 0;
	bCanCombo = true;
	bIsPerformingAirAttack = false;
	bCanAirAttack = true;
}
void UHeroCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateCurrentAttackCollision();
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}
	if (APaperZDCharacter_SpriteHero* OwnerHero = Cast<APaperZDCharacter_SpriteHero>(OwnerCharacter.Get()))
	{
		if (OwnerHero && OwnerHero->OnActionWillInterrupt.IsBound())
		{
			OwnerHero->OnActionWillInterrupt.RemoveDynamic(this, &UHeroCombatComponent::HandleActionInterrupt);
		}
	}
	Super::EndPlay(EndPlayReason);
}
void UHeroCombatComponent::BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (EnhancedInputComponent && ComboAttackAction)
	{
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this,
		                                   &UHeroCombatComponent::HandleAttackInputTriggered);
	}
}
void UHeroCombatComponent::HandleAttackInputTriggered(const FInputActionValue& Value)
{
	if (!OwnerCharacter.IsValid())
	{
		return;
	}
	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}
	if (MovementComp->IsFalling())
	{
		if (CanAirAttack())
		{
			PerformAirAttack();
		}
	}
	else if (!bIsPerformingAirAttack)
	{
		if (!bCanCombo && ComboCount > 0)
		{
			return;
		}
		if (AttackCooldownTimer.IsValid())
		{
			return;
		}
		PerformGroundCombo();
	}
}
void UHeroCombatComponent::PerformGroundCombo()
{
	bool bStartingNewCombo = (ComboCount == 0);
	ComboCount++;
	bCanCombo = false;
	TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
	if (Listener) { Listener->Execute_OnCombatStateChanged(Listener.GetObject(), ComboCount); }
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ResetComboTimer);
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
	}
	if (bStartingNewCombo && OwnerCharacter.IsValid())
	{
		UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
		if (MoveComp && MoveComp->IsMovingOnGround())
		{
			OnGroundComboStarted.Broadcast();
		}
	}
	if (ComboCount >= CurrentMaxGroundComboCount)
	{
		StartAttackCooldown();
	}
}
void UHeroCombatComponent::PerformAirAttack()
{
	bIsPerformingAirAttack = true;
	bCanAirAttack = false;
	bCanCombo = false;
	ComboCount = 0;
	TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
	if (Listener)
	{
		Listener->Execute_OnAirAttackStateChanged(Listener.GetObject(), true);
		Listener->Execute_OnCombatStateChanged(Listener.GetObject(), ComboCount);
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ResetComboTimer);
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
	}
	if (GetWorld() && CurrentAirAttackCooldownDuration > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AirAttackCooldownTimer, this, &UHeroCombatComponent::OnAirAttackCooldownFinished,
			CurrentAirAttackCooldownDuration, false);
	}
	else
	{
		OnAirAttackCooldownFinished();
	}
}
void UHeroCombatComponent::SpawnSwordBeam()
{
	if (!OwnerCharacter.IsValid() || !OwnerSpriteComponent.IsValid() || !CurrentSwordBeamClass || !GetWorld())
	{
		return;
	}
	float DirectionMultiplier = OwnerSpriteComponent->GetRelativeScale3D().X >= 0.0f ? 1.0f : -1.0f;
	FVector SpawnDirection = OwnerCharacter->GetActorForwardVector() * DirectionMultiplier;
	FVector OwnerLocation = OwnerSpriteComponent->GetComponentLocation();
	FRotator OwnerRotation = OwnerCharacter->GetActorRotation();
	FVector FinalOffset = CurrentSwordBeamSpawnOffset;
	FinalOffset.X *= DirectionMultiplier;
	FinalOffset = OwnerRotation.RotateVector(FinalOffset);
	FVector SpawnLocation = OwnerLocation + FinalOffset;
	FRotator SpawnRotation = SpawnDirection.Rotation();
	FActorSpawnParameters SpawnParams;
	APawn* OwnerPawn = Cast<APawn>(OwnerCharacter.Get());
	SpawnParams.Owner = OwnerPawn;
	SpawnParams.Instigator = OwnerPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASwordBeamProjectile* NewProjectile = GetWorld()->SpawnActor<ASwordBeamProjectile>(
		CurrentSwordBeamClass, SpawnLocation, SpawnRotation, SpawnParams
	);
	if (NewProjectile)
	{
		NewProjectile->InitializeProjectile(
			SpawnDirection, CurrentSwordBeamInitialSpeed, CurrentSwordBeamDamage,
			CurrentSwordBeamLifeSpan, OwnerCharacter.Get()
		);
	}
}
void UHeroCombatComponent::StartAttackCooldown()
{
	if (GetWorld() && CurrentGroundAttackCooldownDuration > 0)
	{
		if (!AttackCooldownTimer.IsValid())
		{
			GetWorld()->GetTimerManager().SetTimer(
				AttackCooldownTimer, this, &UHeroCombatComponent::OnAttackCooldownFinished,
				CurrentGroundAttackCooldownDuration, false);
		}
	}
	else if (CurrentGroundAttackCooldownDuration <= 0)
	{
		OnAttackCooldownFinished();
	}
}
void UHeroCombatComponent::OnAttackCooldownFinished()
{
	ResetComboState();
}
void UHeroCombatComponent::OnAirAttackCooldownFinished()
{
	bCanAirAttack = true;
}
void UHeroCombatComponent::ResetComboState()
{
	bool bWasInGroundCombo = (ComboCount > 0 && !bIsPerformingAirAttack);
	bool bWasAirAttacking = bIsPerformingAirAttack;
	bool bStateChanged = (ComboCount != 0 || bIsPerformingAirAttack);
	ComboCount = 0;
	bCanCombo = true;
	bIsPerformingAirAttack = false;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ResetComboTimer);
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
	}
	if (bWasInGroundCombo)
	{
		UE_LOG(LogTemp, Log, TEXT("HeroCombatComponent: Broadcasting OnGroundComboEnded due to ResetComboState."));
		OnGroundComboEnded.Broadcast();
	}
	if (bStateChanged)
	{
		TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
		if (Listener)
		{
			Listener->Execute_OnCombatStateChanged(Listener.GetObject(), ComboCount);
			if (bWasAirAttacking)
			{
				Listener->Execute_OnAirAttackStateChanged(Listener.GetObject(), false);
			}
		}
	}
}
void UHeroCombatComponent::OnAttackHit(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OwnerCharacter.IsValid() || !OtherActor || OtherActor == OwnerCharacter.Get() || !OverlappedComponent)
	{
		return;
	}
	float DamageToApply = 0.0f;
	FName HitCompTag = OverlappedComponent->ComponentTags.IsValidIndex(0)
		                   ? OverlappedComponent->ComponentTags[0]
		                   : NAME_None;
	if (HitCompTag != AttackShapeNames::AttackHitBox &&
		HitCompTag != AttackShapeNames::AttackHitCapsule &&
		HitCompTag != AttackShapeNames::ThrustAttackCapsule)
	{
		return;
	}
	if (bIsPerformingAirAttack)
	{
		if (HitCompTag == AttackShapeNames::AttackHitCapsule)
		{
			DamageToApply = CurrentAirAttackMeleeDamage;
			
		}
	}
	else
	{
		DamageToApply = CurrentGroundBaseAttackDamage;
	
	}
	if (DamageToApply > 0)
	{
		AActor* Attacker = OwnerCharacter.Get();
		if (!UCombatGameplayStatics::CanDamageActor(Attacker, OtherActor))
		{
			return;
		}
		if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
		{
			AController* DamageInstigatorController = nullptr;
			if (APawn* OwnerPawn = Cast<APawn>(OwnerCharacter.Get()))
			{
				DamageInstigatorController = OwnerPawn->GetController();
			}
			IDamageable::Execute_ApplyDamage(OtherActor, DamageToApply, OwnerCharacter.Get(),
			                                 DamageInstigatorController, SweepResult);
			APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(Attacker);
			if (Hero)
			{
				Hero->GetRageComponent()->AddRage(10);
			}
		}
	
	}
}
void UHeroCombatComponent::EnableComboInput()
{
	if (OwnerCharacter.IsValid() && OwnerCharacter->GetCharacterMovement() && !OwnerCharacter->GetCharacterMovement()->
		IsFalling() &&
		!bIsPerformingAirAttack && !AttackCooldownTimer.IsValid() && ComboCount < CurrentMaxGroundComboCount)
	{
		bCanCombo = true;
	}
}
void UHeroCombatComponent::CloseComboWindowAndSetupResetTimer()
{
	if (bIsPerformingAirAttack)
	{
		return;
	}
	bCanCombo = false;
	if (ComboCount < CurrentMaxGroundComboCount && !AttackCooldownTimer.IsValid())
	{
		if (GetWorld() && CurrentComboResetDelay > 0)
		{
			GetWorld()->GetTimerManager().SetTimer(ResetComboTimer, this, &UHeroCombatComponent::ResetComboState,
			                                       CurrentComboResetDelay, false);
		}
		else if (GetWorld() && CurrentComboResetDelay <= 0)
		{
			ResetComboState();
		}
	}
}

void UHeroCombatComponent::HandleAnimNotify_SpawnSwordBeam()
{
	SpawnSwordBeam();
}
void UHeroCombatComponent::HandleAnimNotify_AirAttackEnd()
{
	if (bIsPerformingAirAttack)
	{
		bIsPerformingAirAttack = false;
		TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
		if (Listener) { Listener->Execute_OnAirAttackStateChanged(Listener.GetObject(), false); }
	}
}
void UHeroCombatComponent::ActivateAttackCollision(FName ShapeIdentifier, float Duration)
{
	if (Duration <= 0)
	{
		return;
	}
	DeactivateCurrentAttackCollision();
	UPrimitiveComponent* ShapeToActivate = nullptr;
	if (ShapeIdentifier == AttackShapeNames::AttackHitBox && AttackHitBox) { ShapeToActivate = AttackHitBox.Get(); }
	else if (ShapeIdentifier == AttackShapeNames::AttackHitCapsule && AttackHitCapsule)
	{
		ShapeToActivate = AttackHitCapsule.Get();
	}
	else if (ShapeIdentifier == AttackShapeNames::ThrustAttackCapsule && ThrustAttackCapsule)
	{
		ShapeToActivate = ThrustAttackCapsule.Get();
	}
	if (ShapeToActivate)
	{
		ShapeToActivate->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ActiveAttackCollisionShape = ShapeToActivate;
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(AttackCollisionTimer, this,
			                                       &UHeroCombatComponent::DeactivateCurrentAttackCollision, Duration,
			                                       false);
		}
	}
}
void UHeroCombatComponent::DeactivateCurrentAttackCollision()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackCollisionTimer);
	}
	if (ActiveAttackCollisionShape.IsValid())
	{
		ActiveAttackCollisionShape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ActiveAttackCollisionShape = nullptr;
	}
}
TScriptInterface<ICharacterAnimationStateListener> UHeroCombatComponent::GetAnimListener() const
{
	TScriptInterface<ICharacterAnimationStateListener> Listener = nullptr;
	if (IHeroAnimationStateProvider* AnimProvider = Cast<IHeroAnimationStateProvider>(GetOwner()))
	{
		Listener = IHeroAnimationStateProvider::Execute_GetAnimStateListener(GetOwner());
		if (!Listener.GetInterface())
		{
			return nullptr;
		}
		return Listener;
	}
	return nullptr;
}
void UHeroCombatComponent::NotifyLanded()
{
	if (bIsPerformingAirAttack)
	{
		bIsPerformingAirAttack = false;
		TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
		if (Listener) { Listener->Execute_OnAirAttackStateChanged(Listener.GetObject(), false); }
		if (!AttackCooldownTimer.IsValid())
		{
			bCanCombo = true;
		}
	}
}
void UHeroCombatComponent::HandleActionInterrupt()
{
	UE_LOG(LogTemp, Log, TEXT("HeroCombatComponent: HandleActionInterrupt called."));
	if (ComboCount > 0 || bIsPerformingAirAttack || ActiveAttackCollisionShape.IsValid() || GetWorld()->
		GetTimerManager().IsTimerActive(AttackCollisionTimer))
	{
		ResetComboState();
		DeactivateCurrentAttackCollision();
	}
}