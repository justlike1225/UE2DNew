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
#include "Components/HealthComponent.h"
#include "Interfaces/InputBindingComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "DataAssets/CharacterMovementSettingsDA.h"
#include "Engine/Engine.h" 
#include "My2DGameDesign/My2DGameDesign.h"

// --- 引入状态类头文件 ---
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
// --- End State Includes ---


APaperZDCharacter_SpriteHero::APaperZDCharacter_SpriteHero()
{
	PrimaryActorTick.bCanEverTick = true; // <-- 启用 Tick 以便更新状态

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
		MoveComp->MaxAcceleration = 3000.0f; // 这些值应由 ApplyMovementSettings 覆盖
		MoveComp->BrakingDecelerationWalking = 1000.0f; // 这些值应由 ApplyMovementSettings 覆盖
	}

	SetupCamera(); // 相机设置保持在构造函数

	InitialStateClass = UIdleState::StaticClass(); // 设置默认初始状态类
	CurrentState = nullptr; // 初始化当前状态指针
}

// --- 新增: 状态机初始化函数 ---
void APaperZDCharacter_SpriteHero::InitializeStateMachine()
{
	StateInstances.Empty(); // 清空状态实例 Map
	CurrentState = nullptr; // 重置当前状态指针

	if (!InitialStateClass)
	{
		InitialStateClass = UIdleState::StaticClass(); // 确保有初始状态类
	}

	
	GetOrCreateStateInstance<UIdleState>();
	GetOrCreateStateInstance<UWalkingState>();
	GetOrCreateStateInstance<URunningState>();
	GetOrCreateStateInstance<UJumpingState>();
	GetOrCreateStateInstance<UFallingState>();
	GetOrCreateStateInstance<UAttackingState>();
	GetOrCreateStateInstance<UDashingState>();
	GetOrCreateStateInstance<UHurtState>();
	GetOrCreateStateInstance<UDeadState>();
	// 添加其他需要的状态...

	// --- 设置初始状态 ---
	if (InitialStateClass)
	{
		// 从 Map 中查找对应初始类名的实例
		if (TObjectPtr<UHeroStateBase>* FoundState = StateInstances.Find(InitialStateClass))
		{
			CurrentState = *FoundState; // 设置为当前状态
			if (CurrentState)
			{
				CurrentState->OnEnterState(); // 调用初始状态的进入函数
			}
		}
		else
		{
			// 如果找不到预创建的实例（理论上不应发生），尝试使用 Idle 作为后备
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

// --- 修改: BeginPlay ---
void APaperZDCharacter_SpriteHero::BeginPlay()
{
	Super::BeginPlay();

	// 1. 应用和缓存移动设置 (保持)
	ApplyMovementSettings();
	CacheMovementSpeeds();

	// 2. 缓存动画监听器 (保持)
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
    Super::Tick(DeltaTime); // 调用父类 Tick
    
    // 每帧调用当前状态的 Tick 函数
    if (CurrentState)
    {
        CurrentState->TickState(DeltaTime);
    	
    }
	//打印当前状态名称
	if (CurrentState)
	{
		FString StateName = CurrentState->GetClass()->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, TEXT("Current State: ") + StateName);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("Current State: None"));
	}
}


void APaperZDCharacter_SpriteHero::ChangeState(TSubclassOf<UHeroStateBase> NewStateClass)
{
	if (!NewStateClass)
	{
		return; // 不切换到空类
	}

	// 获取新状态的实例 (应已在 InitializeStateMachine 中创建)
	UHeroStateBase* NewState = nullptr;
	if (TObjectPtr<UHeroStateBase>* FoundState = StateInstances.Find(NewStateClass))
	{
		NewState = *FoundState;
	}
	else
	{
		
		return; 
	}

	
	if (!NewState || NewState == CurrentState)
	{
		return;
	}

	
	if (CurrentState)
	{
		CurrentState->OnExitState();
	}


	CurrentState = NewState;
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
	// 视觉翻转逻辑保持在 Hero 类
	const float MoveValue = Value.Get<float>();
	SetDirection(MoveValue);

	// 将移动输入值传递给当前状态处理
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
	if (CurrentState)
	{
		CurrentState->HandleLanded(Hit);
	}
}

void APaperZDCharacter_SpriteHero::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
                                                                    const FVector& PreviousFloorContactNormal,
                                                                    const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta); // 调用父类实现
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

	return ActualDamage; // 返回实际伤害
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


	if (CurrentState)
	{
		CurrentState->HandleDeath();
	}
}

void APaperZDCharacter_SpriteHero::HandleTakeHit(float CurrentHealthVal, float MaxHealthVal)
{
	
	if (CurrentState && CurrentHealthVal < MaxHealthVal)
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
	return FVector::ForwardVector; // 默认
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


void APaperZDCharacter_SpriteHero::ApplyMovementSettings()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}
	if (MovementSettings)
	{
		MoveComp->MaxWalkSpeed = MovementSettings->MaxWalkSpeed;
		MoveComp->MaxAcceleration = MovementSettings->MaxAcceleration;
		MoveComp->GroundFriction = MovementSettings->GroundFriction;
		MoveComp->BrakingDecelerationWalking = MovementSettings->BrakingDecelerationWalking;
		MoveComp->JumpZVelocity = MovementSettings->JumpZVelocity;
		MoveComp->AirControl = MovementSettings->AirControl;
		MoveComp->GravityScale = MovementSettings->GravityScale;
		
	}

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
			CachedRunSpeed = CachedWalkSpeed * 2.5f; // 估算
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
			MoveComp->MaxWalkSpeed = CachedWalkSpeed; // 应用初始行走速度
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
		if (OtherTeamId != TeamId) { return ETeamAttitude::Hostile; } // 假设非友即敌
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
				// Log Warning
			}
		}
	}
}


void APaperZDCharacter_SpriteHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 绑定基础动作到 Hero 的处理函数 (这些函数会委托给状态)
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

		// 绑定组件的输入 (保持)
		TArray<UActorComponent*> Components;
		GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			if (Component == HealthComponent) continue; // HealthComponent 不绑定输入

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

}


void APaperZDCharacter_SpriteHero::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 解绑 HealthComponent 委托
	if (HealthComponent)
	{
		if (HealthComponent->OnDeath.IsBound()) HealthComponent->OnDeath.RemoveDynamic(this, &APaperZDCharacter_SpriteHero::HandleDeath);
		if (HealthComponent->OnHealthChanged.IsBound()) HealthComponent->OnHealthChanged.RemoveDynamic(this, &APaperZDCharacter_SpriteHero::HandleTakeHit);
	}

    // 清理状态实例
    for (auto& Pair : StateInstances)
    {
        if (Pair.Value && IsValid(Pair.Value)) // 增加 IsValid 检查
        {
            Pair.Value->MarkAsGarbage();
        }
    }
    StateInstances.Empty();
    CurrentState = nullptr;

  

	Super::EndPlay(EndPlayReason);
}

