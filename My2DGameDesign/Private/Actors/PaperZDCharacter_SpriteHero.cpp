// 文件路径: My2DGameDesign/Private/Actors/PaperZDCharacter_SpriteHero.cpp

#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "EnhancedInputSubsystems.h"         // 用于输入系统
#include "EnhancedInputComponent.h"          // 用于绑定输入动作
#include "GameFramework/CharacterMovementComponent.h" // 获取移动组件
#include "PaperFlipbookComponent.h"          // 获取 Sprite 组件
#include "GameFramework/PlayerController.h"    // 获取玩家控制器
#include "PaperZDAnimInstance.h"             // PaperZD 动画实例基类
#include "PaperZDAnimationComponent.h"       // PaperZD 动画组件
#include "Camera/CameraComponent.h"            // 相机组件
#include "Components/DashComponent.h"          // 冲刺组件
#include "Components/AfterimageComponent.h"    // 残影组件
#include "Components/HeroCombatComponent.h"    // 战斗组件
#include "Interfaces/InputBindingComponent.h"  // 输入绑定接口
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h" // 动画状态监听接口
#include "InputMappingContext.h"             // 输入映射上下文
#include "DataAssets/CharacterMovementSettingsDA.h" // <--- 包含运动设置数据资产头文件

// 构造函数
APaperZDCharacter_SpriteHero::APaperZDCharacter_SpriteHero()
{
	// 基本设置
	PrimaryActorTick.bCanEverTick = false; // 通常不需要角色 Tick

	// 创建核心组件
	AfterimageComponent = CreateDefaultSubobject<UAfterimageComponent>(TEXT("AfterimageComponent"));
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	CombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("CombatComponent"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	// 设置移动组件的基础属性 (这些通常不放在数据资产里，是角色的基础行为方式)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false; // 2D游戏不自动旋转角色朝向移动方向
		MoveComp->RotationRate = FRotator(0.f, 0.f, 0.f); // 旋转速率为0

		// 约束在2D平面 (Y=0)
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));

		// 可以在这里设置一些不常变的默认值，但会被数据资产覆盖
		MoveComp->MaxAcceleration = 3000.0f; // 默认加速度
		MoveComp->BrakingDecelerationWalking = 1000.0f; // 默认制动减速度
	}

	// 设置相机
	SetupCamera();

	// 初始化状态变量
	bIsWalking = false;
	bIsRunning = false;
	bIsCanJump = false;
	// CachedWalkSpeed 和 CachedRunSpeed 会在 BeginPlay 中根据数据资产初始化
}

// BeginPlay: 在游戏开始时执行初始化
void APaperZDCharacter_SpriteHero::BeginPlay()
{
	Super::BeginPlay(); // 调用父类的 BeginPlay

	// 1. 应用数据资产中的运动设置
	ApplyMovementSettings();
	// 2. 缓存数据资产中的速度值，方便后续使用
	CacheMovementSpeeds();
	// 3. 初始化动画状态监听器引用
	if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent())
	{
		UPaperZDAnimInstance* BaseAnimInstance = AnimComp->GetAnimInstance();
		if (BaseAnimInstance)
		{
			// 将获取到的 AnimInstance 转换为接口类型并存储
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
	if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround())
	{
		bIsCanJump = true;
	}
	else
	{
		bIsCanJump = false;
	}

	// 5. 初始化动画实例的状态变量 (如果需要立即同步)
	if (AnimationStateListener)
	{
		AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
		if (CombatComponent)
		{
			AnimationStateListener->Execute_OnCombatStateChanged(AnimationStateListener.GetObject(), CombatComponent->GetComboCount());
		}
		if (DashComponent)
		{
			AnimationStateListener->Execute_OnDashStateChanged(AnimationStateListener.GetObject(), DashComponent->IsDashing());
		}
		// 可能还需要同步跳跃/坠落状态等，取决于你的动画蓝图需求
	}
}

// 应用数据资产中的运动设置到移动组件
void APaperZDCharacter_SpriteHero::ApplyMovementSettings()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ApplyMovementSettings: CharacterMovementComponent is missing on %s!"), *GetNameSafe(this));
		return;
	}

	if (MovementSettings) // 检查数据资产是否有效
	{
		UE_LOG(LogTemp, Log, TEXT("ApplyMovementSettings: Applying MovementSettings DA '%s' to %s"), *GetNameSafe(MovementSettings), *GetNameSafe(this));

		// 从数据资产读取值并应用
		MoveComp->MaxWalkSpeed = MovementSettings->MaxWalkSpeed; // 基础行走速度
		MoveComp->MaxAcceleration = MovementSettings->MaxAcceleration;
		MoveComp->GroundFriction = MovementSettings->GroundFriction;
		MoveComp->BrakingDecelerationWalking = MovementSettings->BrakingDecelerationWalking;
		MoveComp->JumpZVelocity = MovementSettings->JumpZVelocity;
		MoveComp->AirControl = MovementSettings->AirControl;
		MoveComp->GravityScale = MovementSettings->GravityScale;
		// ... 应用你在数据资产中定义的其他属性 ...
	}
	else
	{
		// 如果没有分配数据资产，发出警告，使用组件的默认值或硬编码值
		UE_LOG(LogTemp, Warning, TEXT("ApplyMovementSettings: MovementSettings DataAsset is not assigned to %s. Using default CharacterMovementComponent values or previously set defaults."), *GetNameSafe(this));
		// 这里可以保留组件的默认值，或者设置一套后备的硬编码值
		// 例如: MoveComp->MaxWalkSpeed = 250.0f;
	}
}

// 缓存数据资产中的速度值到成员变量
void APaperZDCharacter_SpriteHero::CacheMovementSpeeds()
{
	if (MovementSettings)
	{
		// 从数据资产读取
		CachedWalkSpeed = MovementSettings->MaxWalkSpeed;
		CachedRunSpeed = MovementSettings->MaxRunSpeed;
		UE_LOG(LogTemp, Log, TEXT("CacheMovementSpeeds: Cached WalkSpeed=%.1f, RunSpeed=%.1f from DA %s for %s"), CachedWalkSpeed, CachedRunSpeed, *GetNameSafe(MovementSettings), *GetNameSafe(this));
	}
	else
	{
		// 没有数据资产时的后备逻辑
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		if (MoveComp)
		{
			// 尝试从移动组件当前值获取行走速度
			CachedWalkSpeed = MoveComp->MaxWalkSpeed;
			// 奔跑速度可以基于行走速度估算或使用硬编码默认值
			CachedRunSpeed = CachedWalkSpeed * 2.5f; // 例子：奔跑速度是行走速度的2.5倍
			UE_LOG(LogTemp, Warning, TEXT("CacheMovementSpeeds: MovementSettings DA not found for %s. Using WalkSpeed=%.1f from MoveComp, estimated RunSpeed=%.1f"), *GetNameSafe(this), CachedWalkSpeed, CachedRunSpeed);
		}
		else
		{
			// 连移动组件都没有（理论上不应发生），使用硬编码后备值
			UE_LOG(LogTemp, Error, TEXT("CacheMovementSpeeds: MovementSettings DA and MoveComp not found for %s! Using hardcoded defaults."), *GetNameSafe(this));
			CachedWalkSpeed = 200.f;
			CachedRunSpeed = 500.f;
		}
	}
}

// 获取面向方向 (接口实现)
FVector APaperZDCharacter_SpriteHero::GetFacingDirection_Implementation() const
{
	if (UPaperFlipbookComponent* SpriteComponent = GetSprite())
	{
		// 通过 Sprite 的 X 轴缩放判断朝向
		bool bFacingRight = SpriteComponent->GetRelativeScale3D().X >= 0.0f;
		return bFacingRight ? FVector::ForwardVector : -FVector::ForwardVector; // 返回标准化方向向量
	}
	// 如果获取不到 Sprite，默认朝前
	return FVector::ForwardVector;
}

// 广播动作中断事件 (接口实现)
void APaperZDCharacter_SpriteHero::BroadcastActionInterrupt_Implementation()
{
	OnActionWillInterrupt.Broadcast(); // 触发委托
}

// 获取动画状态监听器 (接口实现)
TScriptInterface<ICharacterAnimationStateListener> APaperZDCharacter_SpriteHero::GetAnimStateListener_Implementation() const
{
	return AnimationStateListener; // 返回缓存的监听器
}

// 初始化移动参数 (现在主要用于重置状态)
void APaperZDCharacter_SpriteHero::InitializeMovementParameters()
{
	// 重置内部状态变量
	bIsWalking = false;
	bIsRunning = false;
	bIsCanJump = false; // 可能需要根据当前是否在地面重新判断

	// 确保基础移动组件设置（虽然构造函数已做，这里是双重保险）
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
	if (Camera && RootComponent) // 确保相机和根组件都有效
	{
		Camera->SetupAttachment(RootComponent); // 附加到根组件
		// 设置相对位置和旋转 (这些值可以根据你的游戏调整)
		Camera->SetRelativeLocation(FVector(0.0f, 150.0f, 50.0f)); // 示例位置
		Camera->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // 示例旋转，俯视角度
		// 设置投影模式为正交
		Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
		// 设置正交视图宽度 (影响视野范围)
		Camera->OrthoWidth = 600.0f; // 示例宽度
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
	// 尝试将对方 Actor 转换为实现了队伍接口的对象
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
	if (OtherTeamAgent)
	{
		// 获取对方的队伍 ID
		FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();
		// 如果队伍 ID 相同，则为友方
		if (OtherTeamId == TeamId)
		{
			return ETeamAttitude::Friendly;
		}
		// 否则视为敌方 (可以根据需要添加中立判断逻辑)
		return ETeamAttitude::Hostile;
	}
	// 如果对方没有实现队伍接口，视为中立
	return ETeamAttitude::Neutral;
}

// 当控制器改变时调用 (例如，玩家控制了一个新的 Pawn)
void APaperZDCharacter_SpriteHero::NotifyControllerChanged()
{
	Super::NotifyControllerChanged(); // 调用父类实现

	// 获取新的玩家控制器和本地玩家
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;

	// 如果是本地玩家在控制
	if (LocalPlayer)
	{
		// 获取增强输入子系统
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			// 清除旧的输入映射
			Subsystem->ClearAllMappings();
			// 添加我们为玩家配置的输入映射上下文
			if (PlayerMappingContext)
			{
				Subsystem->AddMappingContext(PlayerMappingContext, 0); // 0 是优先级
			}
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("NotifyControllerChanged: PlayerMappingContext is not set on %s! Input will not work."), *GetNameSafe(this));
            }
		}
	}
}

// 设置玩家输入组件 (绑定输入动作)
void APaperZDCharacter_SpriteHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 尝试将输入组件转换为增强输入组件
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 绑定跳跃动作
		if (JumpAction)
		{
			// 按下时触发 OnJumpStarted
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APaperZDCharacter_SpriteHero::OnJumpStarted);
			// 松开时触发 OnJumpCompleted
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &APaperZDCharacter_SpriteHero::OnJumpCompleted);
		}
		// 绑定奔跑动作
		if (RunAction)
		{
			// 按住时持续触发 OnRunTriggered
			EnhancedInput->BindAction(RunAction, ETriggerEvent::Triggered, this, &APaperZDCharacter_SpriteHero::OnRunTriggered);
			// 松开时触发 OnRunCompleted
			EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &APaperZDCharacter_SpriteHero::OnRunCompleted);
		}
		// 绑定移动动作
		if (MoveAction)
		{
			// 按住时持续触发 OnMoveTriggered
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APaperZDCharacter_SpriteHero::OnMoveTriggered);
			// 松开时触发 OnMoveCompleted
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &APaperZDCharacter_SpriteHero::OnMoveCompleted);
		}

		// 遍历角色身上的所有组件
		TArray<UActorComponent*> Components;
		GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			// 检查组件是否实现了输入绑定接口 (例如 HeroCombatComponent, DashComponent)
			IInputBindingComponent* InputBinder = Cast<IInputBindingComponent>(Component);
			if (InputBinder)
			{
				// 让实现了接口的组件自己去绑定它关心的输入动作
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
	bIsCanJump = true; // 落地后可以跳跃
	// 通知战斗组件角色已落地 (可能需要中断空中攻击等)
	if (CombatComponent)
	{
		CombatComponent->NotifyLanded();
	}
}

// 当角色从平台边缘走下时调用 (重写基类虚函数)
void APaperZDCharacter_SpriteHero::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta);
	bIsCanJump = false; // 离开地面，不能跳跃
}

// --- 输入动作处理函数 ---

// 跳跃键按下
void APaperZDCharacter_SpriteHero::OnJumpStarted(const FInputActionValue& Value)
{
	if (!bIsCanJump) { return; } // 不能跳跃则返回
	bIsCanJump = false;          // 跳跃后暂时不能再跳
	OnActionWillInterrupt.Broadcast(); // 广播中断事件，让其他组件(如攻击)知道要中断
	Jump();                      // 执行引擎内置的跳跃逻辑
	// 通知动画状态监听器，请求播放跳跃动画
	if (AnimationStateListener)
    {
        AnimationStateListener->Execute_OnJumpRequested(AnimationStateListener.GetObject());
    }
}

// 跳跃键松开
void APaperZDCharacter_SpriteHero::OnJumpCompleted(const FInputActionValue& Value)
{
	StopJumping(); // 执行引擎内置的停止跳跃逻辑 (影响跳跃高度)
}

// 移动键按住 (持续触发)
void APaperZDCharacter_SpriteHero::OnMoveTriggered(const FInputActionValue& Value)
{
	// 获取输入轴的值 (通常是 -1.0 到 1.0)
	const float MoveValue = Value.Get<float>();
	bool bWasWalking = bIsWalking; // 记录之前的行走状态

	// 如果输入值不接近零
	if (FMath::Abs(MoveValue) > KINDA_SMALL_NUMBER)
	{
		bIsWalking = true; // 标记为正在行走
		SetDirection(MoveValue); // 根据输入方向设置角色视觉朝向
		// 向角色当前面向的方向添加移动输入
		AddMovementInput(GetActorForwardVector(), MoveValue);
	}
	else // 输入值为零
	{
		bIsWalking = false; // 标记为停止行走
	}

	// 如果行走状态发生变化，通知动画状态监听器
	if (bIsWalking != bWasWalking && AnimationStateListener)
	{
		AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
	}
}

// 移动键松开
void APaperZDCharacter_SpriteHero::OnMoveCompleted(const FInputActionValue& Value)
{
	bool bWasWalking = bIsWalking;
	bool bWasRunning = bIsRunning; // 同时记录之前的奔跑状态
	bIsWalking = false; // 停止行走
	bIsRunning = false; // 停止移动也意味着停止奔跑

	// 如果行走或奔跑状态发生变化，通知动画状态监听器
	if ((bWasWalking != bIsWalking || bWasRunning != bIsRunning) && AnimationStateListener)
	{
		AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
	}

	// 停止移动后，确保移动组件的最大速度重置为基础行走速度
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CachedWalkSpeed; // 使用缓存的行走速度
	}
}

// 奔跑键按住 (持续触发)
void APaperZDCharacter_SpriteHero::OnRunTriggered(const FInputActionValue& Value)
{
	bool bWasRunning = bIsRunning;
	// 只有在行走且当前没有在奔跑时，才能开始奔跑
	if (bIsWalking && !bIsRunning)
	{
		bIsRunning = true; // 标记为正在奔跑
		// 设置移动组件的最大速度为奔跑速度
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->MaxWalkSpeed = CachedRunSpeed; // 使用缓存的奔跑速度
        }

		// 如果奔跑状态发生变化，通知动画状态监听器
		if (bIsRunning != bWasRunning && AnimationStateListener)
		{
			AnimationStateListener->Execute_OnIntentStateChanged(AnimationStateListener.GetObject(), bIsWalking, bIsRunning);
		}
	}
}

// 奔跑键松开
void APaperZDCharacter_SpriteHero::OnRunCompleted(const FInputActionValue& Value)
{
	bool bWasRunning = bIsRunning;
	if (bIsRunning) // 如果之前在奔跑
	{
		bIsRunning = false; // 标记为停止奔跑
		// 如果角色仍在行走，将移动组件的最大速度恢复为行走速度
		if (bIsWalking) // 检查是否仍在行走
        {
            if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
            {
                 MoveComp->MaxWalkSpeed = CachedWalkSpeed; // 使用缓存的行走速度
            }
        }
        // 如果此时 bIsWalking 是 false (即同时松开了移动键和奔跑键)，
        // 速度会在 OnMoveCompleted 中被重置为 CachedWalkSpeed。

		// 如果奔跑状态发生变化，通知动画状态监听器
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
		// 根据输入方向确定目标 X 轴缩放符号 (1.0 或 -1.0)
		// 如果输入接近零，保持当前缩放
		float TargetSign = (Direction > KINDA_SMALL_NUMBER) ? 1.0f : ((Direction < -KINDA_SMALL_NUMBER) ? -1.0f : FMath::Sign(CurrentScaleX));

        // 获取当前X缩放的绝对值，如果为0则用1替代
        float AbsScaleX = FMath::Abs(CurrentScaleX);
        if (FMath::IsNearlyZero(AbsScaleX)) { AbsScaleX = 1.0f; }

        // 计算目标X缩放值
        float TargetScaleX = AbsScaleX * TargetSign;

		// 只有在目标缩放值与当前值显著不同时才应用更改，避免不必要的更新
		if (!FMath::IsNearlyEqual(CurrentScaleX, TargetScaleX))
		{
			FVector CurrentScale = SpriteComponent->GetRelativeScale3D();
			SpriteComponent->SetRelativeScale3D(FVector(TargetScaleX, CurrentScale.Y, CurrentScale.Z));
		}
	}
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("SetDirection: GetSprite() returned null for %s."), *GetNameSafe(this));
    }
}