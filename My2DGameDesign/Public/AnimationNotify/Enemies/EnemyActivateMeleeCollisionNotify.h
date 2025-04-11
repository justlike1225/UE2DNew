#pragma once
#include "CoreMinimal.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "EnemyActivateMeleeCollisionNotify.generated.h"

class UEnemyMeleeAttackComponent;

UCLASS(DisplayName="Enemy: Activate Melee Collision")
class MY2DGAMEDESIGN_API UEnemyActivateMeleeCollisionNotify : public UPaperZDAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify | Melee Collision")
	FName ShapeIdentifier = FName("MeleeNormal");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify | Melee Collision", meta = (ClampMin = "0.01"))
	float Duration = 0.15f;

protected:
	virtual void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance) const override;
};
