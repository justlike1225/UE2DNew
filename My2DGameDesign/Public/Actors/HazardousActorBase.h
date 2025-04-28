#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "TimerManager.h"
#include "HazardousActorBase.generated.h"


class UBoxComponent;
class UPaperSpriteComponent;
class APaperZDCharacter_SpriteHero;

UCLASS(Abstract, Blueprintable)
class MY2DGAMEDESIGN_API AHazardousActorBase : public AActor
{
	GENERATED_BODY()

public:
	AHazardousActorBase();

protected:
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPrimitiveComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperSpriteComponent> SpriteComp;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard Settings")
	float DamageAmount = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard Settings",
		meta=(ClampMin="0.01", ToolTip="Time in seconds between damage applications while overlapping."))
	float DamageInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard Settings"
	)
	bool bDamageOnInitialOverlap = true;


	UPROPERTY(Transient)
	TWeakObjectPtr<APaperZDCharacter_SpriteHero> OverlappingHeroPtr;


	FTimerHandle DamageTimerHandle;


	UFUNCTION()
	virtual void OnHazardOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                  const FHitResult& SweepResult);


	UFUNCTION()
	virtual void OnHazardOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UFUNCTION()
	virtual void ApplyPeriodicDamage();
};
