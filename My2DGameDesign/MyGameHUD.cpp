
// MyGameHUD.cpp
#include "MyGameHUD.h"
#include "Blueprint/UserWidget.h" // 需要包含 UserWidget 头文件
#include "GameFramework/PlayerController.h" // 需要 PlayerController

void AMyGameHUD::BeginPlay()
{
	Super::BeginPlay();

	// 检查我们是否在编辑器中指定了要使用的控件蓝图类
	if (PlayerHUDWidgetClass)
	{
		// 获取拥有此 HUD 的玩家控制器
		APlayerController* PC = GetOwningPlayerController();
		if (PC)
		{
			// 创建控件实例
			CurrentPlayerHUDWidget = CreateWidget<UUserWidget>(PC, PlayerHUDWidgetClass);

			// 检查控件是否成功创建
			if (CurrentPlayerHUDWidget)
			{
				// 将控件添加到屏幕视口
				CurrentPlayerHUDWidget->AddToViewport();
				UE_LOG(LogTemp, Log, TEXT("Player HUD widget added to viewport."));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create Player HUD widget."));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerHUDWidgetClass is not set in AMyGameHUD defaults!"));
	}
}