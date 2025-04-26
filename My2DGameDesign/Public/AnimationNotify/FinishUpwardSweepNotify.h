// My2DGameDesign/Public/AnimationNotify/FinishUpwardSweepNotify.h
#pragma once
#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "FinishUpwardSweepNotify.generated.h"

UCLASS(DisplayName="Hero: Finish Upward Sweep Skill")
class MY2DGAMEDESIGN_API UFinishUpwardSweepNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};