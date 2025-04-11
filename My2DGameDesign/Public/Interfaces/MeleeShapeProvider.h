#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MeleeShapeProvider.generated.h"

class UPrimitiveComponent;

UINTERFACE(MinimalAPI, Blueprintable)
class UMeleeShapeProvider : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IMeleeShapeProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Melee Shape Provider")
	UPrimitiveComponent* GetMeleeShapeComponent(FName ShapeIdentifier) const;
};
