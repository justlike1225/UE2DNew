#pragma once
#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "EnemyResetMeleeStateNotify.generated.h"

class UEvilCreatureAnimInstance;

UCLASS(DisplayName="Enemy: Reset Melee State")
class MY2DGAMEDESIGN_API UEnemyResetMeleeStateNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

protected:
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};
