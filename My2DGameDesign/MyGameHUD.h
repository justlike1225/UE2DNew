// MyGameHUD.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyGameHUD.generated.h"

class UUserWidget; // 前向声明

UCLASS()
class MY2DGAMEDESIGN_API AMyGameHUD : public AHUD
{
	GENERATED_BODY()

protected:
	/** 我们要在屏幕上显示的玩家 HUD 控件蓝图的类 */
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> PlayerHUDWidgetClass; // 用来在编辑器中指定 WBP_PlayerHUD

	/** 保存创建出来的 HUD Widget 实例，方便以后访问 (可选) */
	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentPlayerHUDWidget;
	/** 处理怒气值变化的函数，将被绑定到 RageComponent 的委托上 */
	UFUNCTION() // UFUNCTION() 是动态绑定所必需的
	void HandleRageChanged(float CurrentRage, float MaxRage);
	/** 当游戏开始时调用 */
	virtual void BeginPlay() override;
};