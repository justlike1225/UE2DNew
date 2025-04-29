
// MyPlayerController.cpp
#include "Controller/MyPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/InputSettings.h" // For FInputModeGameAndUI / FInputModeGameOnly

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 添加输入映射上下文
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
         // 确保先清除可能存在的旧映射，或者根据需要调整优先级
         // Subsystem->ClearAllMappings(); // 谨慎使用，可能会清除其他映射
         if (DefaultMappingContext)
         {
             Subsystem->AddMappingContext(DefaultMappingContext, 0); // 优先级 0
         }
    }
}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (PauseAction)
        {
            EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AMyPlayerController::TogglePauseMenu);
        }
    }
}

void AMyPlayerController::TogglePauseMenu()
{
    // 如果暂停菜单当前可见，则恢复游戏
    if (PauseMenuWidgetInstance && PauseMenuWidgetInstance->IsInViewport())
    {
        ResumeGame();
    }
    // 否则，暂停游戏并显示菜单
    else
    {
        PauseGame();
    }
}

void AMyPlayerController::PauseGame()
{
    if (!PauseMenuWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PauseMenuWidgetClass not set in MyPlayerController defaults!"));
        return;
    }

    // 如果实例已存在但不可见（理论上不该发生，除非逻辑复杂），先处理掉
    if (PauseMenuWidgetInstance && !PauseMenuWidgetInstance->IsInViewport())
    {
         PauseMenuWidgetInstance->RemoveFromParent(); // 或者直接 Destroy
         PauseMenuWidgetInstance = nullptr;
    }

    // 创建实例并添加到视口
    if (!PauseMenuWidgetInstance) // 确保没有重复创建
    {
         PauseMenuWidgetInstance = CreateWidget<UUserWidget>(this, PauseMenuWidgetClass);
    }

    if (PauseMenuWidgetInstance && !PauseMenuWidgetInstance->IsInViewport())
    {
        PauseMenuWidgetInstance->AddToViewport(10); // 设置 ZOrder 较高，确保在最前

        // 暂停游戏
        UGameplayStatics::SetGamePaused(GetWorld(), true);

        // 设置输入模式为 GameAndUI，允许UI交互，但不完全锁定游戏输入（可选，也可选UIOnly）
        FInputModeGameAndUI InputModeData;
        InputModeData.SetWidgetToFocus(PauseMenuWidgetInstance->TakeWidget()); // 让UI获得焦点
        InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 不锁定鼠标
        // InputModeData.SetHideCursorDuringCapture(false); // 确保光标可见
        SetInputMode(InputModeData);

        // 显示鼠标光标
        SetShowMouseCursor(true);

         UE_LOG(LogTemp, Log, TEXT("Game Paused"));
    }
}

void AMyPlayerController::ResumeGame()
{
    if (PauseMenuWidgetInstance)
    {
        PauseMenuWidgetInstance->RemoveFromParent();
        PauseMenuWidgetInstance = nullptr; // 清除引用

        // 恢复游戏
        UGameplayStatics::SetGamePaused(GetWorld(), false);

        // 恢复输入模式为 GameOnly
        FInputModeGameOnly InputModeData;
        SetInputMode(InputModeData);

        // 隐藏鼠标光标
        SetShowMouseCursor(false);

        UE_LOG(LogTemp, Log, TEXT("Game Resumed"));
    }
}