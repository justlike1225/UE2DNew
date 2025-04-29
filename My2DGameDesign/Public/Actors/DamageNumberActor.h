
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h" 
#include "Templates/SubclassOf.h"    
#include "DamageNumberActor.generated.h"

class UUserWidget; 

UCLASS(Blueprintable) 
class MY2DGAMEDESIGN_API ADamageNumberActor : public AActor
{
	GENERATED_BODY()

public:
	ADamageNumberActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> DamageWidgetComp; 

	
	UPROPERTY(EditDefaultsOnly, Category = "Damage Number")
	TSubclassOf<UUserWidget> DamageWidgetClass;

	
	UPROPERTY(EditDefaultsOnly, Category = "Damage Number")
	float LifeSpan = 1.0f;

public:
	
	UFUNCTION(BlueprintCallable, Category = "Damage Number")
	void SetDamageText(int32 DamageAmount,FColor TextColor );

	
	
};