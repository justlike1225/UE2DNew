// MyPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h" // 需要 UserWidget
#include "MyPlayerController.generated.h"

class UInputMappingContext; // Forward declare
class UInputAction;       // Forward declare

UCLASS()
class MY2DGAMEDESIGN_API AMyPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    // 输入相关
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext; // 指向你的 PlayerMappingContext

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> PauseAction; // 指向你的 IA_Pause

    // Pause Menu 相关
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass; // 在蓝图默认值里指定 WBP_PauseMenu

    UPROPERTY(Transient) // 不需要保存
    TObjectPtr<UUserWidget> PauseMenuWidgetInstance;

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // 切换暂停状态的函数
    UFUNCTION()
    void TogglePauseMenu();

    // 暂停游戏并显示菜单 (私有辅助函数)
    void PauseGame();


public:
    // 恢复游戏 (公开，以便 WBP_PauseMenu 可以调用)
    UFUNCTION(BlueprintCallable, Category = "Game")
    void ResumeGame();
    virtual void OnPossess(APawn* InPawn) override;
};
