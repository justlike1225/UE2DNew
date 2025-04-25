// 文件路径: My2DGameDesign/Private/Actors/PaperZDCharacter_SpriteHero.cpp

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
#include "Components/HeroCombatComponent.h"
#include "Components/HealthComponent.h" // <--- 包含 HealthComponent 头文件
#include "Interfaces/InputBindingComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h" // <--- 包含监听器接口
#include "DataAssets/CharacterMovementSettingsDA.h"
#include "Engine/Engine.h" // <--- 包含 Engine.h 用于 GEngine->AddOnScreenDebugMessage

// 构造函数
APaperZDCharacter_SpriteHero::APaperZDCharacter_SpriteHero()
{
	// 基本设置
	PrimaryActorTick.bCanEverTick = false;

	// 创建核心组件
	AfterimageComponent = CreateDefaultSubobject<UAfterimageComponent>(TEXT("AfterimageComponent"));
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	CombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("CombatComponent"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));


	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));


	// 设置移动组件的基础属性
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

	// 设置相机
	SetupCamera();

	// 初始化状态变量
	bIsWalking = false;
	bIsRunning = false;
	bIsCanJump = false;
}

void APaperZDCharacter_SpriteHero::NotifyHurtRecovery()
{
	// 如果确实处于硬直状态，则解除
	if (bIsIncapacitated)
	{
		bIsIncapacitated = false;
		UE_LOG(LogTemp, Log, TEXT("%s: Incapacitated state ended (Notified by AnimInstance)."), *GetNameSafe(this));
	}
}


// BeginPlay: 在游戏开始时执行初始化
void APaperZDCharacter_SpriteHero::BeginPlay()
{
	Super::BeginPlay();

	// 1. 应用数据资产中的运动设置
	ApplyMovementSettings();
	// 2. 缓存数据资产中的速度值
	CacheMovementSpeeds();
	// 3. 初始化动画状态监听器引用
	if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent())
	{
		UPaperZDAnimInstance* BaseAnimInstance = AnimComp->GetAnimInstance();
		if (BaseAnimInstance)
		{
			AnimationStateListener = TScriptInterface<ICharacterAnimationStateListener>(BaseAnimInstance);
			if (!AnimationStateListener)
			{
				UE_LOG(LogTemp, Warning, TEXT("BeginPlay: AnimInstance on %s does not implement ICharacterAnimationStateListener!"), *GetNameSafe(this));
			}
		}
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("BeginPlay: Could not get AnimInstance from AnimationComponent on %s."), *GetNameSafe(this));
        }
	}
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("BeginPlay: Could not get AnimationComponent on %s."), *GetNameSafe(this));
    }

	// 4. 初始化跳跃状态
	if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround()) { bIsCanJump = true; } else { bIsCanJump = false; }

	// 5. 初始化动画实例的状态变量 (如果需要立即同步)
	if (AnimationStateListener)
	{
		AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
		if (CombatComponent) { AnimationStateListener->Execute_OnCombatStateChanged(AnimationStateListener.GetObject(), CombatComponent->GetComboCount()); }
		if (DashComponent) { AnimationStateListener->Execute_OnDashStateChanged(AnimationStateListener.GetObject(), DashComponent->IsDashing()); }
		// 注意: 此时不应同步 Hurt 或 Dead 状态，它们由 HealthComponent 事件驱动
	}

	// 6. 绑定战斗组件的委托
	if (CombatComponent)
	{
		CombatComponent->OnGroundComboStarted.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleComboStarted);
		CombatComponent->OnGroundComboEnded.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleComboEnded);
		UE_LOG(LogTemp, Log, TEXT("SpriteHero: Bound to CombatComponent delegates."));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("SpriteHero: CombatComponent is NULL in BeginPlay, cannot bind delegates.")); }

	// --- 新增：绑定 HealthComponent 的委托 ---
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleDeath);
		HealthComponent->OnHealthChanged.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleTakeHit);
		UE_LOG(LogTemp, Log, TEXT("SpriteHero: Bound to HealthComponent delegates."));
	}
	else { UE_LOG(LogTemp, Error, TEXT("SpriteHero: HealthComponent is NULL in BeginPlay, cannot bind delegates!")); }

	// 确保初始状态正确
	bMovementInputBlocked = false;
}

// 应用数据资产中的运动设置
void APaperZDCharacter_SpriteHero::ApplyMovementSettings()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ApplyMovementSettings: CharacterMovementComponent is missing on %s!"), *GetNameSafe(this));
		return;
	}
	if (MovementSettings)
	{
		UE_LOG(LogTemp, Log, TEXT("ApplyMovementSettings: Applying MovementSettings DA '%s' to %s"), *GetNameSafe(MovementSettings), *GetNameSafe(this));
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
		UE_LOG(LogTemp, Warning, TEXT("ApplyMovementSettings: MovementSettings DataAsset is not assigned to %s. Using default CharacterMovementComponent values or previously set defaults."), *GetNameSafe(this));
	}
}

// 缓存数据资产中的速度值到成员变量
void APaperZDCharacter_SpriteHero::CacheMovementSpeeds()
{
	if (MovementSettings)
	{
		CachedWalkSpeed = MovementSettings->MaxWalkSpeed;
		CachedRunSpeed = MovementSettings->MaxRunSpeed;
		UE_LOG(LogTemp, Log, TEXT("CacheMovementSpeeds: Cached WalkSpeed=%.1f, RunSpeed=%.1f from DA %s for %s"), CachedWalkSpeed, CachedRunSpeed, *GetNameSafe(MovementSettings), *GetNameSafe(this));
	}
	else
	{
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		if (MoveComp)
		{
			CachedWalkSpeed = MoveComp->MaxWalkSpeed;
			CachedRunSpeed = CachedWalkSpeed * 2.5f; // 估算
			UE_LOG(LogTemp, Warning, TEXT("CacheMovementSpeeds: MovementSettings DA not found for %s. Using WalkSpeed=%.1f from MoveComp, estimated RunSpeed=%.1f"), *GetNameSafe(this), CachedWalkSpeed, CachedRunSpeed);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CacheMovementSpeeds: MovementSettings DA and MoveComp not found for %s! Using hardcoded defaults."), *GetNameSafe(this));
			CachedWalkSpeed = 200.f;
			CachedRunSpeed = 500.f;
		}
	}
	// 同时将初始速度应用到移动组件 (如果此时速度为0)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        if (FMath::IsNearlyZero(MoveComp->MaxWalkSpeed))
        {
             MoveComp->MaxWalkSpeed = CachedWalkSpeed;
        }
    }
}

// 获取面向方向 (接口实现)
FVector APaperZDCharacter_SpriteHero::GetFacingDirection_Implementation() const
{
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		bool bFacingRight = SpriteComponent->GetRelativeScale3D().X >= 0.0f;
		return bFacingRight ? FVector::ForwardVector : -FVector::ForwardVector;
	}
	return FVector::ForwardVector; // 默认
}

// 广播动作中断事件 (接口实现)
void APaperZDCharacter_SpriteHero::BroadcastActionInterrupt_Implementation()
{
	OnActionWillInterrupt.Broadcast();
}

// 获取动画状态监听器 (接口实现)
TScriptInterface<ICharacterAnimationStateListener> APaperZDCharacter_SpriteHero::GetAnimStateListener_Implementation() const
{
	// 直接返回 BeginPlay 中缓存的 Listener
	if (!AnimationStateListener)
	{
		// 如果 BeginPlay 时缓存失败，这里可以尝试再次获取，但不推荐频繁调用
		UE_LOG(LogTemp, Warning, TEXT("GetAnimStateListener_Implementation: AnimationStateListener is null!"));
	}
	return AnimationStateListener;
}

// 初始化移动参数 (现在主要用于重置状态)
void APaperZDCharacter_SpriteHero::InitializeMovementParameters()
{
	bIsWalking = false;
	bIsRunning = false;
	if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround()) { bIsCanJump = true; } else { bIsCanJump = false; }

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->RotationRate = FRotator(0.f, 0.f, 0.f);
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
	}
}

// 设置相机 (辅助函数)
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
        if(!Camera) UE_LOG(LogTemp, Warning, TEXT("SetupCamera: Camera component is missing on %s."), *GetNameSafe(this));
        if(!RootComponent) UE_LOG(LogTemp, Warning, TEXT("SetupCamera: RootComponent is missing on %s."), *GetNameSafe(this));
    }
}

// 获取队伍关系 (接口实现)
ETeamAttitude::Type APaperZDCharacter_SpriteHero::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
	if (OtherTeamAgent)
	{
		FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();
		// 假设玩家 TeamId=0, 敌人 TeamId=1
		if (OtherTeamId == TeamId) { return ETeamAttitude::Friendly; } // 自己队伍（理论上不太可能，除非有友方单位）
        // 这里需要根据你的阵营规则决定。通常玩家对非玩家（假设敌人 TeamId != 0）是敌对。
        // 简化：如果 ID 不同，则视为敌对
        if (OtherTeamId != TeamId) { return ETeamAttitude::Hostile; }
	}
	// 无法判断阵营，视为中立
	return ETeamAttitude::Neutral;
}

// 当控制器改变时调用
void APaperZDCharacter_SpriteHero::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (LocalPlayer)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->ClearAllMappings();
			if (PlayerMappingContext) { Subsystem->AddMappingContext(PlayerMappingContext, 0); }
            else { UE_LOG(LogTemp, Warning, TEXT("NotifyControllerChanged: PlayerMappingContext is not set on %s! Input will not work."), *GetNameSafe(this)); }
		}
	}
}

// 设置玩家输入组件 (绑定输入动作)
void APaperZDCharacter_SpriteHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 绑定基础移动输入
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

		// 遍历组件，让实现了 IInputBindingComponent 的组件自己绑定输入
		TArray<UActorComponent*> Components;
		GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			// 排除 HealthComponent，因为它不需要绑定输入
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
         UE_LOG(LogTemp, Error, TEXT("SetupPlayerInputComponent: PlayerInputComponent is not an UEnhancedInputComponent on %s! Enhanced Input will not work."), *GetNameSafe(this));
    }
}

// 角色落地时调用
void APaperZDCharacter_SpriteHero::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bIsCanJump = true;
	if (CombatComponent) { CombatComponent->NotifyLanded(); }
}

// 当角色从平台边缘走下时调用
void APaperZDCharacter_SpriteHero::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta);
	bIsCanJump = false;
}

// --- 输入动作处理函数 ---

// 跳跃键按下
void APaperZDCharacter_SpriteHero::OnJumpStarted(const FInputActionValue& Value)
{
	// --- 修改检查 ---
	if (HealthComponent && HealthComponent->IsDead()) return;
	if (bIsIncapacitated) return; // <--- 检查角色自身的硬直状态
	// --- 修改结束 ---

	if (!bIsCanJump) { return; }
	bIsCanJump = false;
	OnActionWillInterrupt.Broadcast(); // 广播中断事件 (例如打断攻击)
	Jump();                      // 执行引擎跳跃

	// 通知动画实例播放跳跃动画 (这部分不变)
	TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimStateListener_Implementation();
	if (Listener) { Listener->Execute_OnJumpRequested(Listener.GetObject()); }
}
// 跳跃键松开
void APaperZDCharacter_SpriteHero::OnJumpCompleted(const FInputActionValue& Value)
{
	StopJumping(); // 停止跳跃 (影响高度)
}

// 移动键按住 (持续触发)
void APaperZDCharacter_SpriteHero::OnMoveTriggered(const FInputActionValue& Value)
{
	if (HealthComponent && HealthComponent->IsDead()) return;
	if (bIsIncapacitated) return;
   

	if (bMovementInputBlocked) { return; } // 如果输入被阻止
	const float MoveValue = Value.Get<float>();
	bool bWasWalking = bIsWalking;
	if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER)
	{
		bIsWalking = true;
		SetDirection(MoveValue); // 设置视觉朝向
		AddMovementInput(GetActorForwardVector(), MoveValue); // 添加移动输入
	} else { bIsWalking = false; }
	if (bIsWalking != bWasWalking && AnimationStateListener)
	{
		AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
	}
}

// 移动键松开
void APaperZDCharacter_SpriteHero::OnMoveCompleted(const FInputActionValue& Value)
{
	bool bWasWalking = bIsWalking;
	bool bWasRunning = bIsRunning;
	bIsWalking = false;
	bIsRunning = false; // 停止移动也意味着停止奔跑
	if ((bWasWalking != bIsWalking || bWasRunning != bIsRunning) && AnimationStateListener)
	{
		AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
	}
	// 确保速度重置为行走速度
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement()) { MoveComp->MaxWalkSpeed = CachedWalkSpeed; }
}

void APaperZDCharacter_SpriteHero::OnRunTriggered(const FInputActionValue& Value)
{
	// --- 修改检查 ---
	if (HealthComponent && HealthComponent->IsDead()) return;
	if (bIsIncapacitated) return; // <--- 检查角色自身的硬直状态
	if (bMovementInputBlocked) return; // 保留攻击期间的移动阻止检查
	// --- 修改结束 ---

	bool bWasRunning = bIsRunning;
	if (bIsWalking && !bIsRunning) // 只有在行走且未奔跑时才能开始奔跑
	{
		bIsRunning = true;
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement()) { MoveComp->MaxWalkSpeed = CachedRunSpeed; }
		// 通知动画实例 (这部分依然需要，用于更新视觉状态)
		TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimStateListener_Implementation();
		if (bIsRunning != bWasRunning && Listener)
		{
			Listener->Execute_OnIntentStateChanged(Listener.GetObject(), bIsWalking, bIsRunning);
		}
	}
}

// 奔跑键松开
void APaperZDCharacter_SpriteHero::OnRunCompleted(const FInputActionValue& Value)
{
	bool bWasRunning = bIsRunning;
	if (bIsRunning)
	{
		bIsRunning = false;
		// 如果仍在行走，恢复行走速度
		if (bIsWalking)
        {
            if (UCharacterMovementComponent* MoveComp = GetCharacterMovement()) { MoveComp->MaxWalkSpeed = CachedWalkSpeed; }
        }
        // 如果此时 bIsWalking 是 false，速度会在 OnMoveCompleted 中重置
		if (bIsRunning != bWasRunning && AnimationStateListener)
		{
			AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
		}
	}
}

// 设置角色视觉朝向 (辅助函数)
void APaperZDCharacter_SpriteHero::SetDirection(float Direction) const
{
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		float CurrentScaleX = SpriteComponent->GetRelativeScale3D().X;
		float TargetSign = (Direction > KINDA_SMALL_NUMBER) ? 1.0f : ((Direction < -KINDA_SMALL_NUMBER) ? -1.0f : FMath::Sign(CurrentScaleX));
        float AbsScaleX = FMath::Abs(CurrentScaleX);
        if (FMath::IsNearlyZero(AbsScaleX)) { AbsScaleX = 1.0f; } // 防止缩放为0
        float TargetScaleX = AbsScaleX * TargetSign;
		if (!FMath::IsNearlyEqual(CurrentScaleX, TargetScaleX))
		{
			FVector CurrentScale = SpriteComponent->GetRelativeScale3D();
			SpriteComponent->SetRelativeScale3D(FVector(TargetScaleX, CurrentScale.Y, CurrentScale.Z));
		}
	}
     else { UE_LOG(LogTemp, Warning, TEXT("SetDirection: GetSprite() returned null for %s."), *GetNameSafe(this)); }
}

// EndPlay 中解绑委托
void APaperZDCharacter_SpriteHero::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 解绑战斗组件委托
	if (CombatComponent)
	{
		 if (CombatComponent->OnGroundComboStarted.IsBound()) CombatComponent->OnGroundComboStarted.RemoveDynamic(this, &APaperZDCharacter_SpriteHero::HandleComboStarted);
		 if (CombatComponent->OnGroundComboEnded.IsBound()) CombatComponent->OnGroundComboEnded.RemoveDynamic(this, &APaperZDCharacter_SpriteHero::HandleComboEnded);
	}
	// --- 新增：解绑 HealthComponent 的委托 ---
	if (HealthComponent)
	{
		if (HealthComponent->OnDeath.IsBound()) HealthComponent->OnDeath.RemoveDynamic(this, &APaperZDCharacter_SpriteHero::HandleDeath);
		if (HealthComponent->OnHealthChanged.IsBound()) HealthComponent->OnHealthChanged.RemoveDynamic(this, &APaperZDCharacter_SpriteHero::HandleTakeHit);
	}
	Super::EndPlay(EndPlayReason);
}

void APaperZDCharacter_SpriteHero::HandleComboStarted()
{
	bMovementInputBlocked = true;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement()) { MoveComp->Velocity.X = 0.f; } // 停止水平移动
}

void APaperZDCharacter_SpriteHero::HandleComboEnded()
{
	
	bMovementInputBlocked = false;
}
float APaperZDCharacter_SpriteHero::ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser, AController* InstigatorController, const FHitResult& HitResult)
{
	if (!HealthComponent || HealthComponent->IsDead())
	{
		return 0.0f;
	}

	float ActualDamage = HealthComponent->TakeDamage(DamageAmount, DamageCauser, InstigatorController);
	UE_LOG(LogTemp, Log, TEXT("%s took %.1f actual damage from %s."), *GetNameSafe(this), ActualDamage, *GetNameSafe(DamageCauser));

	if (ActualDamage > 0.f && !HealthComponent->IsDead())
	{
		// --- 强化中断处理 ---
		bool bShouldInterrupt = true; // 假设受击总是中断
		if (bShouldInterrupt)
		{
			UE_LOG(LogTemp, Log, TEXT("ApplyDamage: Interrupting action due to taking damage."));

			// 1. 立即设置硬直状态
			bIsIncapacitated = true;
			// 2. 立即解除移动锁定 (硬直优先级更高)
			bMovementInputBlocked = false; // 直接设置，确保移动输入检查通过（虽然硬直本身会阻止移动）

			if (CombatComponent)
			{
				
				BroadcastActionInterrupt_Implementation(); // 确保这个调用能可靠地重置所有攻击状态
			}
         
			TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimStateListener_Implementation();
			if (Listener)
			{
				FVector HitDirection = FVector::ZeroVector;
				if (DamageCauser) { HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal(); }
				else if (HitResult.IsValidBlockingHit()) { HitDirection = -HitResult.ImpactNormal; }

				Listener->Execute_OnTakeHit(Listener.GetObject(), ActualDamage, HitDirection, bShouldInterrupt);
				UE_LOG(LogTemp, Verbose, TEXT("ApplyDamage: Notified Animation Listener OnTakeHit."));
			}
			else { UE_LOG(LogTemp, Warning, TEXT("ApplyDamage: Could not get CharacterAnimationStateListener to notify OnTakeHit.")); }
		}
	}

	return ActualDamage;
}



void APaperZDCharacter_SpriteHero::HandleDeath(AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("Hero %s has died! Killed by %s."), *GetNameSafe(this), *GetNameSafe(Killer));

	// 1. 禁用输入
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		DisableInput(PC);
		UE_LOG(LogTemp, Log, TEXT("Hero input disabled."));
	}

	// 2. 停止移动并禁用碰撞
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}
    SetActorEnableCollision(false); // 禁用 Actor 碰撞

	// 3. --- 通知动画实例进入死亡状态 ---
	TScriptInterface<ICharacterAnimationStateListener> Listener = GetAnimStateListener_Implementation();
	if (Listener)
	{
		Listener->Execute_OnDeathState(Listener.GetObject(), Killer); // <--- 调用接口
		UE_LOG(LogTemp, Log, TEXT("HandleDeath: Notified Animation Listener OnDeathState."));
	}
	else { UE_LOG(LogTemp, Warning, TEXT("HandleDeath: Could not get CharacterAnimationStateListener to notify OnDeathState.")); }

}


void APaperZDCharacter_SpriteHero::HandleTakeHit(float CurrentHealthVal, float MaxHealthVal)
{
	

	// 调试信息
	if (GEngine)
	{
		// 只在受伤且未死亡时打印
		if (CurrentHealthVal < MaxHealthVal && HealthComponent && !HealthComponent->IsDead())
		{
			// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("Hero Took Hit! Health: %.0f / %.0f"), CurrentHealthVal, MaxHealthVal));
		}
	}
}