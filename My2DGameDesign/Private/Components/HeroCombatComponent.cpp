// My2DGameDesign/Private/Components/HeroCombatComponent.cpp

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
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "TimerManager.h"
#include "Actors/SwordBeamProjectile.h"
#include "GameFramework/PlayerController.h"

// 构造函数: 创建碰撞体子对象
UHeroCombatComponent::UHeroCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ComboCount = 0;
	bCanCombo = true;
	bIsPerformingAirAttack = false;
	bCanAirAttack = true;
	bWantsInitializeComponent = true; // 确保 InitializeComponent 会被调用

	// 创建碰撞体
	AttackHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitbox"));
	if (AttackHitBox) AttackHitBox->ComponentTags.Add(AttackShapeNames::AttackHitBox);

	AttackHitCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("AttackHitCapsule"));
	if (AttackHitCapsule) AttackHitCapsule->ComponentTags.Add(AttackShapeNames::AttackHitCapsule);

	ThrustAttackCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ThrustAttackCapsule"));
	if (ThrustAttackCapsule) ThrustAttackCapsule->ComponentTags.Add(AttackShapeNames::ThrustAttackCapsule);
}

// 组件初始化: 附加和配置碰撞体基础属性（除 Enabled 状态）
void UHeroCombatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwnerCharacter = Cast<APaperZDCharacter>(GetOwner());

	if (OwnerCharacter.IsValid())
	{
		OwnerSpriteComponent = OwnerCharacter->GetSprite();

		if (!OwnerSpriteComponent.IsValid())
		{
			UE_LOG(LogTemp, Error,
			       TEXT(
				       "HeroCombatComponent::InitializeComponent: OwnerSpriteComponent is NOT VALID! Attachment will fail."
			       ));
			// 注意：这里不 return，以便后续 BeginPlay 中仍能尝试设置 NoCollision
		}
		else
		{
			// 绑定中断委托
			if (APaperZDCharacter_SpriteHero* OwnerHero = Cast<APaperZDCharacter_SpriteHero>(OwnerCharacter.Get()))
			{
				OwnerHero->OnActionWillInterrupt.AddDynamic(this, &UHeroCombatComponent::HandleActionInterrupt);
			}

			// 附加所有碰撞体到 Sprite
			FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);

			// 配置 AttackHitBox
			if (AttackHitBox)
			{
				AttackHitBox->AttachToComponent(OwnerSpriteComponent.Get(), AttachmentRules);
				AttackHitBox->SetRelativeLocation(FVector(22.0f, 0.0f, 0.0f));
				AttackHitBox->SetBoxExtent(FVector(15.0f, 20.0f, 18.0f));
				ConfigureAttackCollisionComponent(AttackHitBox); // 配置Profile, Response等
				AttackHitBox->OnComponentBeginOverlap.AddDynamic(this, &UHeroCombatComponent::OnAttackHit);
			}
			else { UE_LOG(LogTemp, Error, TEXT("HeroCombatComponent::InitializeComponent: AttackHitBox is NULL!")); }


			// 配置 AttackHitCapsule
			if (AttackHitCapsule)
			{
				AttackHitCapsule->AttachToComponent(OwnerSpriteComponent.Get(), AttachmentRules);
				AttackHitCapsule->SetRelativeLocation(FVector(11.23f, 0.0f, 9.02f));
				AttackHitCapsule->SetRelativeRotation(FRotator(46.0f, 0.0f, 0.0f));
				AttackHitCapsule->SetCapsuleSize(12.0f, 27.0f);
				ConfigureAttackCollisionComponent(AttackHitCapsule); // 配置Profile, Response等
				AttackHitCapsule->OnComponentBeginOverlap.AddDynamic(this, &UHeroCombatComponent::OnAttackHit);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("HeroCombatComponent::InitializeComponent: AttackHitCapsule is NULL!"));
			}


			// 配置 ThrustAttackCapsule
			if (ThrustAttackCapsule)
			{
				ThrustAttackCapsule->AttachToComponent(OwnerSpriteComponent.Get(), AttachmentRules);
				ThrustAttackCapsule->SetRelativeLocation(FVector(48.0f, 0.0f, 0.0f));
				ThrustAttackCapsule->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
				ThrustAttackCapsule->SetCapsuleSize(7.0f, 27.0f);
				ConfigureAttackCollisionComponent(ThrustAttackCapsule); // 配置Profile, Response等
				ThrustAttackCapsule->OnComponentBeginOverlap.AddDynamic(this, &UHeroCombatComponent::OnAttackHit);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("HeroCombatComponent::InitializeComponent: ThrustAttackCapsule is NULL!"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error,
		       TEXT(
			       "HeroCombatComponent::InitializeComponent: Failed to cast GetOwner() ('%s') to APaperZDCharacter! Component cannot function correctly."
		       ), *GetNameSafe(GetOwner()));
	}
}

// 配置单个攻击碰撞体的通用属性（不包括启用状态）
void UHeroCombatComponent::ConfigureAttackCollisionComponent(UPrimitiveComponent* CollisionComp, FName ProfileName)
{
	if (CollisionComp)
	{
		// 注意：不在这里设置 SetCollisionEnabled
		CollisionComp->SetCollisionProfileName(ProfileName);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		if (OwnerCharacter.IsValid()) CollisionComp->IgnoreActorWhenMoving(OwnerCharacter.Get(), true);
		CollisionComp->CanCharacterStepUpOn = ECB_No;
		CollisionComp->SetGenerateOverlapEvents(true); // 显式确保开启重叠事件生成
	}
}


// BeginPlay: 设置初始碰撞状态和读取配置
void UHeroCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// --- 在 BeginPlay 开头强制设置碰撞为 NoCollision ---
	// 这是为了解决初始化后期被蓝图或其他系统意外覆盖回 QueryOnly 的问题
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
	// --- 强制设置结束 ---


	// 读取配置 DataAsset
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
		UE_LOG(LogTemp, Log, TEXT("HeroCombatComponent: Combat settings loaded from DA %s"),
		       *CombatSettings->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HeroCombatComponent: CombatSettings DA is not assigned! Using default values."));
	}

	// 确保初始状态正确
	ComboCount = 0;
	bCanCombo = true;
	bIsPerformingAirAttack = false;
	bCanAirAttack = true;
}

// EndPlay: 清理计时器和委托
void UHeroCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 先关闭可能激活的碰撞体
	DeactivateCurrentAttackCollision();
	// 清理所有计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}
	// 解绑委托
	if (APaperZDCharacter_SpriteHero* OwnerHero = Cast<APaperZDCharacter_SpriteHero>(OwnerCharacter.Get()))
	{
		if (OwnerHero && OwnerHero->OnActionWillInterrupt.IsBound())
		{
			OwnerHero->OnActionWillInterrupt.RemoveDynamic(this, &UHeroCombatComponent::HandleActionInterrupt);
		}
	}

	Super::EndPlay(EndPlayReason);
}

// --- 接口实现 ---
void UHeroCombatComponent::BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (EnhancedInputComponent && ComboAttackAction)
	{
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this,
		                                   &UHeroCombatComponent::HandleAttackInputTriggered);
		// UE_LOG(LogTemp, Log, TEXT("HeroCombatComponent: Bound ComboAttackAction ('%s')"), *GetNameSafe(ComboAttackAction)); // 可以保留这个Log用于确认绑定
	}
	else
	{
		if (!EnhancedInputComponent) UE_LOG(LogTemp, Error,
		                                    TEXT(
			                                    "HeroCombatComponent::BindInputActions - EnhancedInputComponent is NULL!"
		                                    ));
		if (!ComboAttackAction) UE_LOG(LogTemp, Warning,
		                               TEXT("HeroCombatComponent::BindInputActions - ComboAttackAction is not assigned!"
		                               ));
	}
}

// --- 输入处理 ---
void UHeroCombatComponent::HandleAttackInputTriggered(const FInputActionValue& Value)
{
	if (!OwnerCharacter.IsValid()) return;

	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	if (!MovementComp) return;

	// 判断空中或地面逻辑
	if (MovementComp->IsFalling())
	{
		if (CanAirAttack())
		{
			PerformAirAttack();
		}
	}
	else if (!bIsPerformingAirAttack) // 地面逻辑
	{
		if (!bCanCombo && ComboCount > 0) return; // 连击窗口已关闭
		if (AttackCooldownTimer.IsValid()) return; // 攻击冷却中

		PerformGroundCombo();
	}
}


// --- 核心逻辑实现 ---
void UHeroCombatComponent::PerformGroundCombo()
{
	ComboCount++;
	bCanCombo = false; // 等待 AnimNotify 开启连击窗口

	TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
	if (Listener) { Listener->Execute_OnCombatStateChanged(Listener.GetObject(), ComboCount); }

	// 清理可能存在的计时器 (防止连击重置和攻击冷却同时生效)
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ResetComboTimer);
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
	}

	UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: PerformGroundCombo - Combo: %d."), ComboCount);

	// 如果达到最大连击数，直接开始攻击冷却
	if (ComboCount >= CurrentMaxGroundComboCount)
	{
		StartAttackCooldown();
	}
	// 否则，等待动画通知来开启连击窗口或关闭窗口并设置重置计时器
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

	// 清理地面攻击相关计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ResetComboTimer);
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
	}

	// 启动空中攻击冷却
	if (GetWorld() && CurrentAirAttackCooldownDuration > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AirAttackCooldownTimer, this, &UHeroCombatComponent::OnAirAttackCooldownFinished,
			CurrentAirAttackCooldownDuration, false);
	}
	else
	{
		OnAirAttackCooldownFinished(); // 如果冷却为0，立即可以再次空攻
	}
}

void UHeroCombatComponent::SpawnSwordBeam()
{
	if (!OwnerCharacter.IsValid() || !OwnerSpriteComponent.IsValid() || !CurrentSwordBeamClass || !GetWorld())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("HeroCombatComponent::SpawnSwordBeam - Cannot spawn projectile due to invalid prerequisites."));
		return;
	}

	float DirectionMultiplier = OwnerSpriteComponent->GetRelativeScale3D().X >= 0.0f ? 1.0f : -1.0f;
	FVector SpawnDirection = OwnerCharacter->GetActorForwardVector() * DirectionMultiplier; // 使用角色朝向
	FVector OwnerLocation = OwnerSpriteComponent->GetComponentLocation(); // 从 Sprite 获取位置更准确
	FRotator OwnerRotation = OwnerCharacter->GetActorRotation(); // 使用角色旋转

	// 应用偏移量, 并考虑角色面向
	FVector FinalOffset = CurrentSwordBeamSpawnOffset;
	FinalOffset.X *= DirectionMultiplier; // X轴偏移根据面向翻转
	FinalOffset = OwnerRotation.RotateVector(FinalOffset); // 应用旋转

	FVector SpawnLocation = OwnerLocation + FinalOffset;
	FRotator SpawnRotation = SpawnDirection.Rotation(); // 抛射物方向

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
		UE_LOG(LogTemp, Log, TEXT("HeroCombatComponent: Spawned Sword Beam '%s'"), *NewProjectile->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HeroCombatComponent: Failed to spawn Sword Beam Actor of class '%s'!"),
		       *GetNameSafe(CurrentSwordBeamClass));
	}
}


// --- 状态重置与冷却 ---
void UHeroCombatComponent::StartAttackCooldown()
{
	if (GetWorld() && CurrentGroundAttackCooldownDuration > 0)
	{
		// 确保不会重复设置冷却计时器
		if (!AttackCooldownTimer.IsValid())
		{
			UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: Starting Ground Attack Cooldown for %.2f seconds"),
			       CurrentGroundAttackCooldownDuration);
			GetWorld()->GetTimerManager().SetTimer(
				AttackCooldownTimer, this, &UHeroCombatComponent::OnAttackCooldownFinished,
				CurrentGroundAttackCooldownDuration, false);
		}
	}
	else if (CurrentGroundAttackCooldownDuration <= 0)
	{
		// 如果冷却时间为0或负数，则立即完成冷却（相当于可以无限连击，除非被ComboCount限制）
		OnAttackCooldownFinished();
	}
}

void UHeroCombatComponent::OnAttackCooldownFinished()
{
	UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: Ground Attack Cooldown Finished."));
	ResetComboState(); // 冷却结束后重置连击状态
}

void UHeroCombatComponent::OnAirAttackCooldownFinished()
{
	bCanAirAttack = true;
	UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: Air Attack Cooldown Finished."));
}

void UHeroCombatComponent::ResetComboState()
{
	bool bWasAirAttacking = bIsPerformingAirAttack;
	bool bComboReset = (ComboCount != 0); // 是否真的需要重置连击计数

	// 重置状态
	ComboCount = 0;
	bCanCombo = true; // 允许开始新的连击
	bIsPerformingAirAttack = false;

	// 清理地面连击相关的计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ResetComboTimer);
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
	}

	// 如果状态确实发生了变化，通知动画实例
	if (bComboReset || bWasAirAttacking)
	{
		TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
		if (Listener)
		{
			if (bComboReset) Listener->Execute_OnCombatStateChanged(Listener.GetObject(), ComboCount);
			if (bWasAirAttacking) Listener->Execute_OnAirAttackStateChanged(Listener.GetObject(), false);
			UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: ResetComboState - Notified AnimInstance."));
		}
	}
}

// 碰撞处理
void UHeroCombatComponent::OnAttackHit(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 检查基本有效性，以及确保不会命中自己
	if (!OwnerCharacter.IsValid() || !OtherActor || OtherActor == OwnerCharacter.Get() || !OverlappedComponent || !
		OtherComp)
	{
		return;
	}

	// 可以增加一个Tag检查或其他方式来确定 OtherActor 是有效的攻击目标（例如敌人）
	// if (!OtherActor->ActorHasTag(FName("Enemy"))) { return; } // 示例

	float DamageToApply = 0.0f;
	FName HitCompTag = NAME_None; // 用于识别是哪个攻击形状命中了

	// 获取命中形状的标签
	if (OverlappedComponent->ComponentHasTag(AttackShapeNames::AttackHitBox)) HitCompTag =
		AttackShapeNames::AttackHitBox;
	else if (OverlappedComponent->ComponentHasTag(AttackShapeNames::AttackHitCapsule)) HitCompTag =
		AttackShapeNames::AttackHitCapsule;
	else if (OverlappedComponent->ComponentHasTag(AttackShapeNames::ThrustAttackCapsule)) HitCompTag =
		AttackShapeNames::ThrustAttackCapsule;

	// 如果命中的不是我们定义的攻击形状之一，忽略
	if (HitCompTag == NAME_None) return;

	// 根据战斗状态决定伤害值
	if (bIsPerformingAirAttack)
	{
		// 假设空中攻击只使用 AttackHitCapsule 造成伤害
		if (HitCompTag == AttackShapeNames::AttackHitCapsule)
		{
			DamageToApply = CurrentAirAttackMeleeDamage;
		}
	}
	else // 地面攻击
	{
		// 假设所有地面攻击都造成相同的伤害
		DamageToApply = CurrentGroundBaseAttackDamage;
	}

	if (DamageToApply > 0)
	{
		AController* DamageInstigatorController = nullptr;
		if (APawn* OwnerPawn = Cast<APawn>(OwnerCharacter.Get()))
		{
			DamageInstigatorController = OwnerPawn->GetController();
		}

		UGameplayStatics::ApplyPointDamage(
			OtherActor, DamageToApply, (SweepResult.ImpactPoint - OwnerCharacter->GetActorLocation()).GetSafeNormal(),
			SweepResult,
			DamageInstigatorController, OwnerCharacter.Get(), UDamageType::StaticClass()
		);

		UE_LOG(LogTemp, Log, TEXT("HeroCombatComponent: Applied %.1f damage to %s using shape %s."), DamageToApply,
		       *OtherActor->GetName(), *HitCompTag.ToString());

		// 可选：如果希望攻击命中一次后就失效，可以在这里调用 DeactivateCurrentAttackCollision();
		// DeactivateCurrentAttackCollision();
	}
}


// --- AnimNotify 调用的函数 ---
void UHeroCombatComponent::EnableComboInput()
{
	if (OwnerCharacter.IsValid() && OwnerCharacter->GetCharacterMovement() && !OwnerCharacter->GetCharacterMovement()->
		IsFalling() &&
		!bIsPerformingAirAttack && !AttackCooldownTimer.IsValid() && ComboCount < CurrentMaxGroundComboCount)
	{
		bCanCombo = true;
		UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: EnableComboInput - Combo window OPENED."));
	}
}

void UHeroCombatComponent::CloseComboWindowAndSetupResetTimer()
{
	if (bIsPerformingAirAttack) return;

	bCanCombo = false; // 关闭连击窗口
	UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: CloseComboWindowAndSetupResetTimer - Combo window CLOSED."));

	// 如果未达到最大连击且不在冷却中，设置连击重置计时器
	if (ComboCount < CurrentMaxGroundComboCount && !AttackCooldownTimer.IsValid())
	{
		if (GetWorld() && CurrentComboResetDelay > 0)
		{
			GetWorld()->GetTimerManager().SetTimer(ResetComboTimer, this, &UHeroCombatComponent::ResetComboState,
			                                       CurrentComboResetDelay, false);
		}
		else if (GetWorld() && CurrentComboResetDelay <= 0)
		{
			ResetComboState(); // 如果延迟无效，立即重置
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
		// 冷却由 PerformAirAttack 启动，这里只标记状态结束
	}
}

// 激活攻击碰撞体
void UHeroCombatComponent::ActivateAttackCollision(FName ShapeIdentifier, float Duration)
{
	// UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: ActivateAttackCollision CALLED with Shape: %s, Duration: %.2f"), *ShapeIdentifier.ToString(), Duration); // 保留这个Verbose日志

	if (Duration <= 0) return;

	// 先尝试关闭上一个激活的碰撞体
	DeactivateCurrentAttackCollision();

	UPrimitiveComponent* ShapeToActivate = nullptr;

	// 根据标识符查找对应的碰撞体组件
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
		ShapeToActivate->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 设置为仅查询
		ActiveAttackCollisionShape = ShapeToActivate; // 记录当前激活的碰撞体

		// 设置定时器，在 Duration 时间后调用 DeactivateCurrentAttackCollision
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(AttackCollisionTimer, this,
			                                       &UHeroCombatComponent::DeactivateCurrentAttackCollision, Duration,
			                                       false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "HeroCombatComponent::ActivateAttackCollision - Could not find or activate shape with identifier '%s'."
		       ), *ShapeIdentifier.ToString());
	}
}

// 关闭当前激活的攻击碰撞体
void UHeroCombatComponent::DeactivateCurrentAttackCollision()
{
	// 清除可能存在的计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackCollisionTimer);
	}

	if (ActiveAttackCollisionShape.IsValid())
	{
		ActiveAttackCollisionShape->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 设置为无碰撞
		// UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: DeactivateCurrentAttackCollision - Deactivated '%s'."), *ActiveAttackCollisionShape->GetName()); // 可以保留这个Verbose日志
		ActiveAttackCollisionShape = nullptr; // 清除引用
	}
}

// --- 辅助函数 ---
TScriptInterface<ICharacterAnimationStateListener> UHeroCombatComponent::GetAnimListener() const
{
	TScriptInterface<ICharacterAnimationStateListener> Listener = nullptr; // 初始化为无效
	if (IHeroAnimationStateProvider* AnimProvider = Cast<IHeroAnimationStateProvider>(GetOwner()))
	{
		Listener = IHeroAnimationStateProvider::Execute_GetAnimStateListener(GetOwner());
		// 再次检查返回的接口是否真的有效
		if (!Listener.GetInterface())
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("HeroCombatComponent: Owner '%s' provided an invalid Animation Listener via interface."),
			       *GetNameSafe(GetOwner()));
			return nullptr; // 确保是无效的
		}
		else { return Listener; } // 返回有效的监听器
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HeroCombatComponent: Owner '%s' does not implement IAnimationStateProvider!"),
		       *GetNameSafe(GetOwner()));
	}
	return nullptr;
}

void UHeroCombatComponent::NotifyLanded()
{
	if (bIsPerformingAirAttack)
	{
		UE_LOG(LogTemp, Verbose, TEXT("HeroCombatComponent: NotifyLanded - Landing detected during Air Attack."));
		bIsPerformingAirAttack = false;
		TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimListener();
		if (Listener) { Listener->Execute_OnAirAttackStateChanged(Listener.GetObject(), false); }
		// 落地后，如果地面攻击不在冷却中，则允许开始连击
		if (!AttackCooldownTimer.IsValid())
		{
			bCanCombo = true;
		}
	}
}

void UHeroCombatComponent::HandleActionInterrupt()
{
	// 如果正在攻击，则重置状态并关闭碰撞
	if (ComboCount > 0 || bIsPerformingAirAttack)
	{
		UE_LOG(LogTemp, Log,
		       TEXT(
			       "HeroCombatComponent: HandleActionInterrupt received during combat. Resetting state and deactivating collision."
		       ));
		ResetComboState();
		DeactivateCurrentAttackCollision();
	}
}
