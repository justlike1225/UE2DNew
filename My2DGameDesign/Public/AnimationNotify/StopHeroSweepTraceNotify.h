#pragma once

#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "StopHeroSweepTraceNotify.generated.h"

UCLASS(DisplayName="Hero: Stop Sweep Trace")
class MY2DGAMEDESIGN_API UStopHeroSweepTraceNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()
protected:
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};