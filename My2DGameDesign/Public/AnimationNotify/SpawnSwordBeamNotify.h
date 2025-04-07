#pragma once

#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "SpawnSwordBeamNotify.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API USpawnSwordBeamNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

public:
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};