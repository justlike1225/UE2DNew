#pragma once
#include "CoreMinimal.h"
#include "EnemyMeleeAttackType.generated.h"
UENUM(BlueprintType)
enum class   EEnemyMeleeAttackType : uint8
{
	Attack1 UMETA(DisplayName="Attack 1"), 
	Attack2 UMETA(DisplayName="Attack 2"), 
	Default UMETA(DisplayName="Default Attack")
};