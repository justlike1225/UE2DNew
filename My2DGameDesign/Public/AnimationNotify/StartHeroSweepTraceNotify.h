#pragma once

#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h" // 继承自标准的 PaperZD Notify
#include "StartHeroSweepTraceNotify.generated.h"

UCLASS(DisplayName="Hero: Start Sweep Trace")
class MY2DGAMEDESIGN_API UStartHeroSweepTraceNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

public:
	/**
	 * 本次攻击判定的总持续时间（秒）。
	 * 这个值将传递给 StartSweepTrace 函数。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notify Settings", meta=(ClampMin="0.01"))
	float TraceDuration = 0.3f; // 默认持续 0.3 秒

protected:
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};