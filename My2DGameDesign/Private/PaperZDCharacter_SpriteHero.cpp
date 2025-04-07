#include "PaperZDCharacter_SpriteHero.h"
#include "EnhancedInputSubsystems.h" // 输入子系统
#include "EnhancedInputComponent.h" // 增强输入组件
#include "GameFramework/CharacterMovementComponent.h" // 角色移动组件
#include "PaperFlipbookComponent.h" // Flipbook 组件
#include "GameFramework/PlayerController.h" // 玩家控制器
#include "PaperZDAnimInstance.h" // PaperZD 动画实例 基类
#include "PaperZDAnimationComponent.h" // PaperZD 动画组件
#include "Camera/CameraComponent.h" // 摄像机组件
#include "Components/BoxComponent.h" // 盒子碰撞体
#include "Components/CapsuleComponent.h" // 胶囊碰撞体
#include "Components/DashComponent.h" // 冲刺组件
#include "Components/AfterimageComponent.h" // 残影组件
#include "Components/HeroCombatComponent.h" // 战斗组件
#include "Interfaces/InputBindingComponent.h" // 输入绑定接口
#include "Interfaces/CharacterAnimationStateListener.h" // 动画状态监听器接口
#include "InputMappingContext.h" // 输入映射上下文
// 构造函数: 创建组件和初始化默认值
APaperZDCharacter_SpriteHero::APaperZDCharacter_SpriteHero()
{
	// 通常角色不需要每帧 Tick，除非有特殊逻辑
	PrimaryActorTick.bCanEverTick = false;

	// --- 创建核心组件 ---
	AfterimageComponent = CreateDefaultSubobject<UAfterimageComponent>(TEXT("AfterimageComponent"));
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	CombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("CombatComponent"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	// --- 初始化函数调用 ---
	InitializeMovementParameters(); // 设置移动相关变量和组件初始值
	
	SetupCamera(); // 创建和配置摄像机
}

// 初始化移动相关参数
void APaperZDCharacter_SpriteHero::InitializeMovementParameters()
{
	// 初始化状态
	bIsWalking = false;
	bIsRunning = false;
	bIsCanJump = false; // 初始在空中或未知状态，不允许跳跃

	// 获取移动组件并设置参数
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed; // 设置初始为行走速度
        MoveComp->bOrientRotationToMovement = false; // 2D游戏通常不根据移动旋转角色
        MoveComp->RotationRate = FRotator(0.f, 0.f, 0.f); // 不需要旋转速率
        MoveComp->MaxAcceleration = 3000.0f; // 最大加速度
		MoveComp->BrakingDecelerationWalking = 1000.0f; // 行走制动减速度

        // 约束移动到 XZ 平面 (对于 Y-up 的 2D 游戏)
        MoveComp->bConstrainToPlane = true;
        MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
        MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
	}
}


// 设置摄像机
void APaperZDCharacter_SpriteHero::SetupCamera()
{
	// Camera 在构造函数中已创建
	Camera->SetupAttachment(RootComponent); // 附加到根组件
	Camera->SetRelativeLocation(FVector(0.0f, 150.0f, 50.0f)); // 设置相对位置
	Camera->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // 设置相对旋转
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic); // 设置为正交投影
	Camera->OrthoWidth = 600.0f; // 设置正交视图宽度
}

// BeginPlay: 初始化组件、状态和缓存监听器
void APaperZDCharacter_SpriteHero::BeginPlay()
{
	Super::BeginPlay();



	// --- 绑定冲刺组件的委托 ---
	if (DashComponent)
	{
		DashComponent->OnDashStarted.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleDashStarted);
		DashComponent->OnDashEnded.AddDynamic(this, &APaperZDCharacter_SpriteHero::HandleDashEnded);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::BeginPlay - DashComponent is null! Cannot bind delegates."), *GetName());
	}

	// --- 获取并缓存动画状态监听器接口 ---
	if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent())
	{
		UPaperZDAnimInstance* BaseAnimInstance = AnimComp->GetAnimInstance();
		if (BaseAnimInstance)
		{
            AnimationStateListener = TScriptInterface<ICharacterAnimationStateListener>(BaseAnimInstance);
            if (AnimationStateListener.GetInterface() && AnimationStateListener.GetObject())
            {
                UE_LOG(LogTemp, Log, TEXT("%s: Found and cached AnimationStateListener on AnimInstance '%s'."),
                       *GetName(), *BaseAnimInstance->GetName());
            }
            else
            {
                 AnimationStateListener = nullptr; // 明确置空
                 UE_LOG(LogTemp, Warning, TEXT("%s: AnimInstance '%s' does NOT implement ICharacterAnimationStateListener interface."),
                       *GetName(), *BaseAnimInstance->GetName());
            }
		}
		else { UE_LOG(LogTemp, Warning, TEXT("%s::BeginPlay - Failed to get AnimInstance from PaperZDAnimationComponent."), *GetName()); }
	}
    else { UE_LOG(LogTemp, Warning, TEXT("%s::BeginPlay - PaperZDAnimationComponent not found."), *GetName()); }


    // --- 初始状态设置 ---
    if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround())
    {
        bIsCanJump = true;
    }
    else
    {
        bIsCanJump = false;
    }

    // --- 推送初始状态给动画实例 ---
    if (AnimationStateListener)
    {
        AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
        if(CombatComponent) { AnimationStateListener->Execute_OnCombatStateChanged(AnimationStateListener.GetObject(), CombatComponent->GetComboCount()); }
        if(DashComponent) { AnimationStateListener->Execute_OnDashStateChanged(AnimationStateListener.GetObject(), DashComponent->IsDashing()); }
    }
}

// NotifyControllerChanged: 处理控制器附加/移除，管理输入映射
void APaperZDCharacter_SpriteHero::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
    ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;

    if (LocalPlayer)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
            UE_LOG(LogTemp, Log, TEXT("%s: Controller changed to PlayerController. Adding Input Mapping Context '%s'."),
                *GetName(), PlayerMappingContext ? *PlayerMappingContext->GetName() : TEXT("NONE"));

			Subsystem->ClearAllMappings(); // 通常建议清除旧的，防止叠加
            if (PlayerMappingContext)
            {
			    Subsystem->AddMappingContext(PlayerMappingContext, 0);
            }
            else { UE_LOG(LogTemp, Warning, TEXT("%s: PlayerMappingContext is not set! Input will not work."), *GetName()); }
		}
        else { UE_LOG(LogTemp, Error, TEXT("%s: Failed to get EnhancedInputLocalPlayerSubsystem."), *GetName()); }
	}
    else // 控制器为空或不是玩家控制器
    {
        
         UE_LOG(LogTemp, Log, TEXT("%s: Controller is not a PlayerController or is null. Input mapping context management skipped/needs review."), *GetName());
    }
}

// SetupPlayerInputComponent: 设置输入绑定，分发给组件
void APaperZDCharacter_SpriteHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Setting up Enhanced Input Bindings..."), *GetName());

		// --- 1. 绑定角色自身处理的输入动作 ---
		bool bBoundAnyCharacterAction = false;
		if (JumpAction) {
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APaperZDCharacter_SpriteHero::OnJumpStarted);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &APaperZDCharacter_SpriteHero::OnJumpCompleted);
			UE_LOG(LogTemp, Verbose, TEXT("   Bound JumpAction ('%s')"), *JumpAction->GetName());
            bBoundAnyCharacterAction = true;
		}
        if (RunAction) {
            EnhancedInput->BindAction(RunAction, ETriggerEvent::Triggered, this, &APaperZDCharacter_SpriteHero::OnRunTriggered);
		    EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &APaperZDCharacter_SpriteHero::OnRunCompleted);
            UE_LOG(LogTemp, Verbose, TEXT("   Bound RunAction ('%s')"), *RunAction->GetName());
             bBoundAnyCharacterAction = true;
        }
        if (MoveAction) {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APaperZDCharacter_SpriteHero::OnMoveTriggered);
		    EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &APaperZDCharacter_SpriteHero::OnMoveCompleted);
             UE_LOG(LogTemp, Verbose, TEXT("   Bound MoveAction ('%s')"), *MoveAction->GetName());
              bBoundAnyCharacterAction = true;
        }
        if (!bBoundAnyCharacterAction)
        {
             UE_LOG(LogTemp, Warning, TEXT("%s: No character-specific Input Actions (Jump, Move, Run) were assigned or bound."), *GetName());
        }

		// --- 2. 查找并调用实现了 InputBindingComponent 接口的组件 ---
		TArray<UActorComponent*> Components;
		GetComponents(Components);
        UE_LOG(LogTemp, Log, TEXT("   Searching for components implementing IInputBindingComponent..."));
        int32 BoundComponentCount = 0;
		for (UActorComponent* Component : Components)
		{
			IInputBindingComponent* InputBinder = Cast<IInputBindingComponent>(Component);
			if (InputBinder)
			{
                UE_LOG(LogTemp, Log, TEXT("      Found component '%s' implementing the interface. Calling BindInputActions..."), *Component->GetName());
				InputBinder->Execute_BindInputActions(Component, EnhancedInput);
                BoundComponentCount++;
			}
		}
         UE_LOG(LogTemp, Log, TEXT("   Found and called BindInputActions on %d component(s)."), BoundComponentCount);
	}
    else
    {
         UE_LOG(LogTemp, Error, TEXT("%s: PlayerInputComponent is not an EnhancedInputComponent! Input binding failed."), *GetName());
    }
}


void APaperZDCharacter_SpriteHero::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit); 

	
	bIsCanJump = true; // 落地后通常可以跳跃


	if (CombatComponent) // 确保战斗组件有效
	{
		
		CombatComponent->NotifyLanded(); // 调用战斗组件的函数
	}
	
	
}
// OnWalkingOffLedge_Implementation: 从边缘掉落
void APaperZDCharacter_SpriteHero::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
                                                                    const FVector& PreviousFloorContactNormal,
                                                                    const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta);
    bool bStateChanged = bIsCanJump;
	bIsCanJump = false;
    if (bStateChanged)
    {
         UE_LOG(LogTemp, Verbose, TEXT("%s Walked off ledge."), *GetName());
         // 可选推送物理状态
    }
}


void APaperZDCharacter_SpriteHero::OnJumpStarted(const FInputActionValue& Value)
{
	

	if (!bIsCanJump) {  return; }
	bIsCanJump = false;
	// --- 广播打断事件 ---
	OnActionWillInterrupt.Broadcast();
	// --- 广播结束 ---
	Jump();
	if (AnimationStateListener) { AnimationStateListener->Execute_OnJumpRequested(AnimationStateListener.GetObject()); }
}

// OnJumpCompleted: 处理跳跃按键松开
void APaperZDCharacter_SpriteHero::OnJumpCompleted(const FInputActionValue& Value)
{
	StopJumping();
}

// OnMoveTriggered: 处理移动输入 (按住)，推送状态
void APaperZDCharacter_SpriteHero::OnMoveTriggered(const FInputActionValue& Value)
{
	const float MoveValue = Value.Get<float>();
	bool bWasWalking = bIsWalking;

	if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER) // 使用阈值判断是否移动
	{
		bIsWalking = true;
		SetDirection(MoveValue);
		AddMovementInput(GetActorForwardVector(), MoveValue);
	}
    else // 输入接近0，视为停止移动意图
    {
       bIsWalking = false;
    }

    // 如果行走状态改变，推送通知
    if (bIsWalking != bWasWalking && AnimationStateListener)
    {
        AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
    }

    // 打断攻击逻辑 (如果需要)
	if ((CombatComponent && CombatComponent->GetComboCount() > 0))
    {
        // 可能需要打断或忽略移动
    }
}

// OnMoveCompleted: 处理移动输入结束 (松开)，推送状态
void APaperZDCharacter_SpriteHero::OnMoveCompleted(const FInputActionValue& Value)
{
    bool bWasWalking = bIsWalking;
    bool bWasRunning = bIsRunning;
	bIsWalking = false;
	bIsRunning = false; // 停止移动时也必须停止奔跑

    // 如果行走或奔跑状态发生改变，推送通知
	if ((bWasWalking != bIsWalking || bWasRunning != bIsRunning) && AnimationStateListener)
	{
        AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
	}

    // 恢复速度
	if (GetCharacterMovement()) { GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; }
}

// OnRunTriggered: 处理奔跑输入 (按住)，推送状态
void APaperZDCharacter_SpriteHero::OnRunTriggered(const FInputActionValue& Value)
{
    bool bWasRunning = bIsRunning;
	if (bIsWalking && !bIsRunning) // 必须在行走时按下奔跑才有效
	{
		bIsRunning = true;
        if (GetCharacterMovement()) { GetCharacterMovement()->MaxWalkSpeed = RunSpeed; }

        // 如果奔跑状态改变，推送通知
        if (bIsRunning != bWasRunning && AnimationStateListener)
        {
            AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
        }
	}
}

// OnRunCompleted: 处理奔跑输入结束 (松开)，推送状态
void APaperZDCharacter_SpriteHero::OnRunCompleted(const FInputActionValue& Value)
{
    bool bWasRunning = bIsRunning;
	if (bIsRunning)
	{
		bIsRunning = false;
		if (bIsWalking && GetCharacterMovement()) // 如果仍在行走，恢复行走速度
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}

        // 如果奔跑状态改变，推送通知
        if (bIsRunning != bWasRunning && AnimationStateListener)
        {
            AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
        }
	}
}


// SetDirection: 根据输入方向翻转 Sprite
void APaperZDCharacter_SpriteHero::SetDirection(float Direction) const
{
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
        float CurrentScaleX = SpriteComponent->GetRelativeScale3D().X;
        // 使用小阈值避免浮点数比较问题
        float TargetScaleX = (Direction > 0.0f) ? 1.0f : -1.0f;

        if (!FMath::IsNearlyEqual(CurrentScaleX, TargetScaleX))
        {
            FVector CurrentScale = SpriteComponent->GetRelativeScale3D();
		    SpriteComponent->SetRelativeScale3D(FVector(TargetScaleX, CurrentScale.Y, CurrentScale.Z));
        }
	}
}


// HandleDashStarted: 响应冲刺组件的 OnDashStarted 委托 (只做日志或特殊处理)
void APaperZDCharacter_SpriteHero::HandleDashStarted()
{
	UE_LOG(LogTemp, Log, TEXT("%s: Received Dash Started signal from DashComponent."), *GetName());
	// 动画状态由推送的 OnDashStateChanged 驱动
}

// HandleDashEnded: 响应冲刺组件的 OnDashEnded 委托 (只做日志或特殊处理)
void APaperZDCharacter_SpriteHero::HandleDashEnded()
{
	UE_LOG(LogTemp, Log, TEXT("%s: Received Dash Ended signal from DashComponent."), *GetName());
    // 动画状态由推送的 OnDashStateChanged 驱动
}

