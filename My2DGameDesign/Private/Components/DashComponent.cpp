
// My2DGameDesign/Private/Components/DashComponent.cpp
#include "Components/DashComponent.h"
#include "DataAssets/HeroDashSettingsDA.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AfterimageComponent.h"
#include "PaperFlipbookComponent.h"
#include "Engine/World.h"
#include "TimerManager.h" // 确保包含TimerManager
#include "EnhancedInputComponent.h" // 包含增强输入头文件
#include "PaperZDCharacter_SpriteHero.h" // 需要角色类来获取 Listener
#include "Interfaces/CharacterAnimationStateListener.h" // 需要监听器接口

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
		OwnerSpriteComponent = OwnerCharacter->FindComponentByClass<UPaperFlipbookComponent>();

		if (!OwnerMovementComponent.IsValid()) { UE_LOG(LogTemp, Error, TEXT("DashComponent: Owner '%s' does not have a CharacterMovementComponent!"), *OwnerCharacter->GetName()); }
        if (!OwnerSpriteComponent.IsValid()) { UE_LOG(LogTemp, Warning, TEXT("DashComponent: Owner '%s' does not have a PaperFlipbookComponent (needed for direction)!"), *OwnerCharacter->GetName()); }
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

	if (OwnerSpriteComponent.IsValid()) { DashDirectionMultiplier = OwnerSpriteComponent->GetRelativeScale3D().X >= 0.0f ? 1.0f : -1.0f; }
    else { DashDirectionMultiplier = 1.0f; UE_LOG(LogTemp, Warning, TEXT("DashComponent::ExecuteDashLogic: No valid sprite found, using default forward direction (1.0).")); }

	PerformDash();
}

// PerformDash: 执行冲刺效果并推送状态
void UDashComponent::PerformDash()
{
	
	if (!OwnerCharacter.IsValid() || !OwnerMovementComponent.IsValid() || !GetWorld()) return;
	// --- 通过 Owning Character 广播打断事件 ---
	APaperZDCharacter_SpriteHero* OwnerHero = Cast<APaperZDCharacter_SpriteHero>(OwnerCharacter.Get());
	if (OwnerHero)
	{
		OwnerHero->OnActionWillInterrupt.Broadcast();
	}
	// --- 广播结束 ---
	// 获取 Listener
    TScriptInterface<ICharacterAnimationStateListener> Listener = nullptr;
    APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwnerCharacter.Get()); // 直接 Cast
    if(Hero) { Listener = Hero->GetAnimationStateListener(); }

    bool bWasDashing = bIsDashing;
	bIsDashing = true;
	bCanDash = false;
    UE_LOG(LogTemp, Log, TEXT("DashComponent: Performing Dash..."));

    // 推送状态变化
    if (Listener && bIsDashing != bWasDashing)
    {
        Listener->Execute_OnDashStateChanged(Listener.GetObject(), true);
    }

	// 保存参数
	OriginalGroundFriction = OwnerMovementComponent->GroundFriction;
	OriginalMaxWalkSpeed = OwnerMovementComponent->MaxWalkSpeed;
	OriginalGravityScale = OwnerMovementComponent->GravityScale;

    // 计算速度和方向
	FVector DashDirection = OwnerCharacter->GetActorForwardVector() * DashDirectionMultiplier;
	DashDirection.Normalize();
	FVector TargetDashVelocity = DashDirection * CurrentDashSpeed;
	TargetDashVelocity.Z = 0.0f;

    // 修改移动组件
	OwnerMovementComponent->GravityScale = 0.0f;
	OwnerMovementComponent->GroundFriction = 0.0f;
    OwnerMovementComponent->StopMovementKeepPathing();
	OwnerMovementComponent->Velocity = TargetDashVelocity;

    // 广播委托
    OnDashStarted.Broadcast();

    // 激活残影
	if (OwnerAfterimageComponent.IsValid()) { OwnerAfterimageComponent->StartSpawning(); }

    // 设置计时器
    GetWorld()->GetTimerManager().SetTimer(DashEndTimer, this, &UDashComponent::EndDash, CurrentDashDuration, false);
    GetWorld()->GetTimerManager().SetTimer(DashCooldownTimer, this, &UDashComponent::ResetDashCooldown, CurrentDashCooldown, false);
}

// EndDash: 结束冲刺效果并推送状态
void UDashComponent::EndDash()
{
	if (!OwnerCharacter.IsValid() || !OwnerMovementComponent.IsValid() || !GetWorld() || !bIsDashing) return;

	// 获取 Listener
    TScriptInterface<ICharacterAnimationStateListener> Listener = nullptr;
    APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OwnerCharacter.Get());
    if(Hero) { Listener = Hero->GetAnimationStateListener(); }

    bool bWasDashing = bIsDashing;
    bIsDashing = false;
    UE_LOG(LogTemp, Log, TEXT("DashComponent: Ending Dash."));

    // 推送状态变化
     if (Listener && bIsDashing != bWasDashing)
    {
        Listener->Execute_OnDashStateChanged(Listener.GetObject(), false);
    }

	// 停止残影
    if (OwnerAfterimageComponent.IsValid()) { OwnerAfterimageComponent->StopSpawning(); }

    // 恢复移动参数
    OwnerMovementComponent->GravityScale = OriginalGravityScale;
	OwnerMovementComponent->GroundFriction = OriginalGroundFriction;
	OwnerMovementComponent->MaxWalkSpeed = OriginalMaxWalkSpeed;

    // 广播委托
    OnDashEnded.Broadcast();
}

// ResetDashCooldown: 冷却结束
void UDashComponent::ResetDashCooldown()
{
    UE_LOG(LogTemp, Log, TEXT("DashComponent: Cooldown Finished. Can dash again."));
	bCanDash = true;
}