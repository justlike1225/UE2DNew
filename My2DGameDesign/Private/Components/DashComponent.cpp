
// My2DGameDesign/Private/Components/DashComponent.cpp
#include "Components/DashComponent.h"
#include "DataAssets/HeroDA/HeroDashSettingsDA.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AfterimageComponent.h"
#include "Engine/World.h"
#include "TimerManager.h" // 确保包含TimerManager
#include "EnhancedInputComponent.h" // 包含增强输入头文件
#include "Interfaces/ActionInterruptSource.h"
#include "Interfaces/HeroAnimationStateProvider.h"
#include "Interfaces/CharacterAnimationStateListener.h" // 需要监听器接口
#include "Interfaces/FacingDirectionProvider.h"

// 构造函数
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

// BeginPlay: 获取引用和配置
void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter.IsValid())
	{
		OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
        OwnerAfterimageComponent = OwnerCharacter->FindComponentByClass<UAfterimageComponent>();
		

		if (!OwnerMovementComponent.IsValid()) { UE_LOG(LogTemp, Error, TEXT("DashComponent: Owner '%s' does not have a CharacterMovementComponent!"), *OwnerCharacter->GetName()); }
		if (!OwnerAfterimageComponent.IsValid()) { UE_LOG(LogTemp, Log, TEXT("DashComponent: Owner '%s' does not have an AfterimageComponent (optional)."), *OwnerCharacter->GetName()); }

		if(OwnerMovementComponent.IsValid())
		{
			OriginalGroundFriction = OwnerMovementComponent->GroundFriction;
			OriginalMaxWalkSpeed = OwnerMovementComponent->MaxWalkSpeed;
            OriginalGravityScale = OwnerMovementComponent->GravityScale;
		}
	}
	else { UE_LOG(LogTemp, Error, TEXT("DashComponent requires its owner to be a Character!")); }

	if (DashSettings)
	{
		CurrentDashSpeed = DashSettings->DashSpeed;
		CurrentDashDuration = DashSettings->DashDuration;
		CurrentDashCooldown = DashSettings->DashCooldown;
        UE_LOG(LogTemp, Log, TEXT("DashComponent: Loaded settings from DA %s (Speed: %.f, Duration: %.2f, Cooldown: %.2f)"),
            *DashSettings->GetName(), CurrentDashSpeed, CurrentDashDuration, CurrentDashCooldown);
	}
	else { UE_LOG(LogTemp, Warning, TEXT("DashComponent: No valid DashSettingsDA assigned! Using default values.")); }
}

// EndPlay: 清理计时器
void UDashComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if(GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DashEndTimer);
        GetWorld()->GetTimerManager().ClearTimer(DashCooldownTimer);
    }
    Super::EndPlay(EndPlayReason);
}

// BindInputActions: 绑定输入
void UDashComponent::BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (EnhancedInputComponent && DashAction)
	{
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &UDashComponent::HandleDashInputTriggered);
		UE_LOG(LogTemp, Log, TEXT("DashComponent: Bound input action '%s' to HandleDashInputTriggered."), *DashAction->GetName());
	}
	else
	{
		if(!DashAction) UE_LOG(LogTemp, Warning, TEXT("DashComponent::BindInputActions: DashAction asset is not assigned in the component properties!"));
		if(!EnhancedInputComponent) UE_LOG(LogTemp, Warning, TEXT("DashComponent::BindInputActions: Received null EnhancedInputComponent!"));
	}
}

// HandleDashInputTriggered: 输入事件的直接响应者
void UDashComponent::HandleDashInputTriggered(const FInputActionValue& Value)
{
	ExecuteDashLogic();
}

// ExecuteDashLogic: 执行冲刺的核心逻辑检查
void UDashComponent::ExecuteDashLogic()
{
	if (!CanDash())
	{
       UE_LOG(LogTemp, Verbose, TEXT("DashComponent::ExecuteDashLogic: Cannot dash (Either cooling down or already dashing)."));
		return;
	}
    if(!OwnerCharacter.IsValid() || !OwnerMovementComponent.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("DashComponent::ExecuteDashLogic: OwnerCharacter or MovementComponent is invalid."));
        return;
    }
	
	//if (OwnerSpriteComponent.IsValid()) { DashDirectionMultiplier = OwnerSpriteComponent->GetRelativeScale3D().X >= 0.0f ? 1.0f : -1.0f; }
  //  else { DashDirectionMultiplier = 1.0f; UE_LOG(LogTemp, Warning, TEXT("DashComponent::ExecuteDashLogic: No valid sprite found, using default forward direction (1.0).")); }

	PerformDash();
}
void UDashComponent::PerformDash()
{
    AActor* OwnerActor = GetOwner();
    // 确保 Owner 和 MovementComponent 有效 (BeginPlay 中应该已经检查过，但再次检查更安全)
    if (!OwnerActor || !OwnerMovementComponent.IsValid() || !GetWorld()) return;

    // --- 1. 获取方向 (通过 IFacingDirectionProvider) ---
    FVector DashDirection = FVector::ForwardVector; // 默认值
    IFacingDirectionProvider* DirectionProvider = Cast<IFacingDirectionProvider>(OwnerActor);
    if (DirectionProvider)
    {
        DashDirection = IFacingDirectionProvider::Execute_GetFacingDirection(OwnerActor);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DashComponent: Owner '%s' does not implement IFacingDirectionProvider. Falling back to ActorForwardVector."), *OwnerActor->GetName());
        DashDirection = OwnerActor->GetActorForwardVector(); // 备用逻辑
    }

    // --- 2. 广播中断 (通过 IActionInterruptSource) ---
    IActionInterruptSource* InterruptSource = Cast<IActionInterruptSource>(OwnerActor);
    if (InterruptSource)
    {
        // 使用 Execute_ 调用更安全，特别是对于蓝图实现
        IActionInterruptSource::Execute_BroadcastActionInterrupt(OwnerActor);
    }
    // else: 如果 Owner 不能广播中断，也无妨，继续执行

    // --- 3. 获取动画监听器 (通过 IAnimationStateProvider) ---
    TScriptInterface<ICharacterAnimationStateListener> Listener = nullptr; // 初始化为无效
    IHeroAnimationStateProvider* AnimProvider = Cast<IHeroAnimationStateProvider>(OwnerActor);
    if (AnimProvider)
    {
        Listener = IHeroAnimationStateProvider::Execute_GetAnimStateListener(OwnerActor);
        // 再次检查返回的接口是否真的有效
        if (!Listener.GetInterface())
        {
             UE_LOG(LogTemp, Warning, TEXT("DashComponent: Owner '%s' provided an invalid Animation Listener via interface."), *OwnerActor->GetName());
             Listener = nullptr; // 确保是无效的
        }
    }
    // else: 如果 Owner 不能提供监听器，也无妨，只是动画状态不会被推送

    // --- 4. 更新自身状态 ---
    bool bWasDashing = bIsDashing;
    bIsDashing = true;
    bCanDash = false;
    UE_LOG(LogTemp, Log, TEXT("DashComponent: Performing Dash in direction %s..."), *DashDirection.ToString());

    // --- 5. 推送动画状态 (如果 Listener 有效) ---
    if (Listener && bIsDashing != bWasDashing)
    {
        Listener->Execute_OnDashStateChanged(Listener.GetObject(), true);
    }

    // --- 6. 执行物理冲刺 (依赖 UCharacterMovementComponent) ---
    OriginalGroundFriction = OwnerMovementComponent->GroundFriction;
    OriginalMaxWalkSpeed = OwnerMovementComponent->MaxWalkSpeed;
    OriginalGravityScale = OwnerMovementComponent->GravityScale;
    FVector TargetDashVelocity = DashDirection.GetSafeNormal() * CurrentDashSpeed;
    OwnerMovementComponent->GravityScale = 0.0f;
    OwnerMovementComponent->GroundFriction = 0.0f;
    OwnerMovementComponent->StopMovementKeepPathing();
    OwnerMovementComponent->Velocity = TargetDashVelocity;

    // --- 7. 广播冲刺开始事件 (给外部，如 AfterimageComponent) ---
    OnDashStarted_Event.Broadcast();

    // --- 8. 设置计时器 ---
    GetWorld()->GetTimerManager().SetTimer(DashEndTimer, this, &UDashComponent::EndDash, CurrentDashDuration, false);
    GetWorld()->GetTimerManager().SetTimer(DashCooldownTimer, this, &UDashComponent::ResetDashCooldown, CurrentDashCooldown, false);
}

void UDashComponent::EndDash()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor || !OwnerMovementComponent.IsValid() || !GetWorld() || !bIsDashing) return;

    // --- 1. 获取动画监听器 (通过 IAnimationStateProvider) ---
    TScriptInterface<ICharacterAnimationStateListener> Listener = nullptr;
    IHeroAnimationStateProvider* AnimProvider = Cast<IHeroAnimationStateProvider>(OwnerActor);
    if (AnimProvider)
    {
        Listener = IHeroAnimationStateProvider::Execute_GetAnimStateListener(OwnerActor);
         if (!Listener.GetInterface()) { Listener = nullptr; } // 确保无效
    }

    // --- 2. 更新自身状态 ---
    bool bWasDashing = bIsDashing;
    bIsDashing = false;
    UE_LOG(LogTemp, Log, TEXT("DashComponent: Ending Dash."));

    // --- 3. 推送动画状态 (如果 Listener 有效) ---
    if (Listener && bIsDashing != bWasDashing)
    {
        Listener->Execute_OnDashStateChanged(Listener.GetObject(), false);
    }

    // --- 4. 广播冲刺结束事件 (给外部，如 AfterimageComponent) ---
    OnDashEnded_Event.Broadcast();

    // --- 5. 恢复物理状态 (依赖 UCharacterMovementComponent) ---
    OwnerMovementComponent->GravityScale = OriginalGravityScale;
    OwnerMovementComponent->GroundFriction = OriginalGroundFriction;
    OwnerMovementComponent->MaxWalkSpeed = OriginalMaxWalkSpeed;

    // --- 6. （可选）广播内部结束委托 ---
    // OnDashEnded.Broadcast(); // 检查这个委托是否仍被内部或其他地方使用

}

// ResetDashCooldown: 冷却结束
void UDashComponent::ResetDashCooldown()
{
    UE_LOG(LogTemp, Log, TEXT("DashComponent: Cooldown Finished. Can dash again."));
	bCanDash = true;
}