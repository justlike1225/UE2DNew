#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AfterImageActor.generated.h"

class UAfterImagePoolSubsystem;
class UPaperFlipbookComponent;
class UPaperFlipbook;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class MY2DGAMEDESIGN_API AAfterImageActor : public AActor
{
	GENERATED_BODY()

public:
	AAfterImageActor();

	void Activate(
		UPaperFlipbook* FlipbookToCopy,
		UMaterialInterface* MaterialToUse,
		float LifeTime,
		const FTransform& SpriteTransform,
		FName InOpacityParamName,
		float InInitialOpacity,
		float InFadeUpdateInterval,
		UAfterImagePoolSubsystem* InOwningPool
	);

	void Deactivate();

	bool IsActive() const { return bIsActive; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sprite")
	UPaperFlipbookComponent* AfterImageSprite;

	UPROPERTY()
	UMaterialInstanceDynamic* MaterialInstanceDynamic = nullptr;

	FTimerHandle FadeTimerHandle;
	FTimerHandle LifetimeTimerHandle;
	float CreationTime = 0.0f;
	float ActorLifeTime = 0.0f;
	FName OpacityParameterName;
	float CurrentInitialOpacity = 1.0f;
	bool bIsActive = false;

	UPROPERTY()
	TWeakObjectPtr<UAfterImagePoolSubsystem> OwningPoolSubsystemPtr;

	UFUNCTION()
	void UpdateFade();

	UFUNCTION()
	void OnLifetimeExpired();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ResetActor();
};
