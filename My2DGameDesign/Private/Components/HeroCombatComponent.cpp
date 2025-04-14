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
#include "GameFramework/PlayerController.h"
#include "Interfaces/Damageable.h"
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
		// 最好确保是在地面上开始的 Combo
		if (MoveComp && MoveComp->IsMovingOnGround())
		{
			UE_LOG(LogTemp, Log, TEXT("HeroCombatComponent: Broadcasting OnGroundComboStarted."));
			OnGroundComboStarted.Broadcast(); // <--- 广播开始事件
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
	bool bStateChanged = (ComboCount != 0 || bIsPerformingAirAttack); // 标记状态是否真的改变了

	// --- 重置核心状态 ---
	ComboCount = 0;
	bCanCombo = true; // 重置后应该可以开始新连击 (除非有冷却)
	bIsPerformingAirAttack = false;

	// --- 清理计时器 ---
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ResetComboTimer);
	
	}


	if (bWasInGroundCombo)
	{
		UE_LOG(LogTemp, Log, TEXT("HeroCombatComponent: Broadcasting OnGroundComboEnded due to ResetComboState."));
		OnGroundComboEnded.Broadcast(); // 这个会通知 Actor 将 bMovementInputBlocked 设为 false
	}

	// --- 通知动画实例 ---
	if (bStateChanged) // 只有在状态确实改变时才通知，避免冗余调用
	{
		TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
		if (Listener)
		{
			// 总是通知 ComboCount 为 0
			Listener->Execute_OnCombatStateChanged(Listener.GetObject(), ComboCount);

			if (bWasAirAttacking) // 如果之前是空袭状态，通知空袭结束
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
	// 1. 基本检查
	if (!OwnerCharacter.IsValid() || !OtherActor || OtherActor == OwnerCharacter.Get() || !OverlappedComponent)
	{
		return;
	}

	// 2. 确定伤害值 (这部分逻辑保持不变)
	float DamageToApply = 0.0f;
	FName HitCompTag = OverlappedComponent->ComponentTags.IsValidIndex(0) ? OverlappedComponent->ComponentTags[0] : NAME_None;

	if (HitCompTag != AttackShapeNames::AttackHitBox &&
		HitCompTag != AttackShapeNames::AttackHitCapsule &&
		HitCompTag != AttackShapeNames::ThrustAttackCapsule)
	{
		return; // 不是已知的攻击形状
	}

	if (bIsPerformingAirAttack)
	{
		if (HitCompTag == AttackShapeNames::AttackHitCapsule) // 假设空袭只用这个胶囊造成伤害
		{
			DamageToApply = CurrentAirAttackMeleeDamage;
			UE_LOG(LogTemp, Log, TEXT("Hero Air Attack Hit %s with %s (Damage: %.1f)"), *OtherActor->GetName(), *HitCompTag.ToString(), DamageToApply);
		}
	}
	else // 地面攻击
	{
		DamageToApply = CurrentGroundBaseAttackDamage; // 地面攻击使用基础伤害
		UE_LOG(LogTemp, Log, TEXT("Hero Ground Attack Hit %s with %s (Damage: %.1f)"), *OtherActor->GetName(), *HitCompTag.ToString(), DamageToApply);
	}

	// 3. 施加伤害 (修改部分)
	if (DamageToApply > 0)
	{
		AActor* Attacker = OwnerCharacter.Get();
		if (!UCombatGameplayStatics::CanDamageActor(Attacker, OtherActor))
		{
			return; // 如果不能伤害，直接返回
		}
		if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
		{
			// 获取伤害施加者的控制器
			AController* DamageInstigatorController = nullptr;
			if (APawn* OwnerPawn = Cast<APawn>(OwnerCharacter.Get())) // 获取 Pawn
			{
				DamageInstigatorController = OwnerPawn->GetController(); // 从 Pawn 获取 Controller
			}

			// 调用接口函数施加伤害
			// 参数：目标Actor, 伤害值, 伤害来源Actor (英雄自己), 来源控制器, HitResult
			IDamageable::Execute_ApplyDamage(OtherActor, DamageToApply, OwnerCharacter.Get(), DamageInstigatorController, SweepResult);
			UE_LOG(LogTemp, Log, TEXT("Applied damage %.1f to %s via IDamageable interface."), DamageToApply, *OtherActor->GetName());

			// 可选：在这里添加击中效果等
		}
		else
		{
			// 如果对方没有实现 IDamageable 接口，则不施加伤害
			UE_LOG(LogTemp, Log, TEXT("Actor %s does not implement IDamageable. No damage applied by HeroCombatComponent."), *OtherActor->GetName());
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
	// 检查是否确实有需要中断的状态
	if (ComboCount > 0 || bIsPerformingAirAttack || ActiveAttackCollisionShape.IsValid() || GetWorld()->GetTimerManager().IsTimerActive(AttackCollisionTimer))
	{
		ResetComboState(); // 重置逻辑状态
		DeactivateCurrentAttackCollision(); // 关闭当前攻击碰撞
	}
}
