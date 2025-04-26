#pragma once

#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "RageDashExecuteMovementNotify.generated.h"

UCLASS(DisplayName="Hero: Execute RageDash Movement") // 在编辑器里显示的名字
class MY2DGAMEDESIGN_API URageDashExecuteMovementNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

protected:
	// 重写接收通知的函数
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};