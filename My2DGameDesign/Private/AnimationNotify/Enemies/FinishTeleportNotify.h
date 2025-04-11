// My2DGameDesign/Public/AnimationNotify/Enemies/FinishTeleportNotify.h
#pragma once

#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "FinishTeleportNotify.generated.h"

UCLASS(DisplayName="Enemy: Finish Teleport State")
class MY2DGAMEDESIGN_API UFinishTeleportNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};