#pragma once

#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "EnableComboNotify.generated.h"

/**
 * @brief 动画通知：用于在动画播放到此帧时，通知战斗组件开启连击输入窗口。
 */
UCLASS()
class MY2DGAMEDESIGN_API UEnableComboNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

public:
	// 当动画播放到包含此通知的帧时，引擎会自动调用这个函数。
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};