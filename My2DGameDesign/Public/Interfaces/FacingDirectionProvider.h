#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Math/Vector.h"
#include "FacingDirectionProvider.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UFacingDirectionProvider : public UInterface
{
	GENERATED_BODY()
};

class MY2DGAMEDESIGN_API IFacingDirectionProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Direction Provider")
	FVector GetFacingDirection() const;
};
