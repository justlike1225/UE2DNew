#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AfterimageComponent.generated.h"
class UAfterImagePoolSubsystem;
class UHeroFXSettingsDA;
class AAfterImageActor;
class UPaperFlipbookComponent;
class UPaperFlipbook;
class UMaterialInterface;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MY2DGAMEDESIGN_API UAfterimageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAfterimageComponent();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UHeroFXSettingsDA> FXSettings;
	UPROPERTY(Transient)
	TObjectPtr<UAfterImagePoolSubsystem> AfterImagePoolSubsystemPtr;
	float CurrentAfterImageInterval = 0.05f;
	float CurrentAfterImageLifetime = 0.3f;
	FName CurrentOpacityParamName = FName("Opacity");
	float CurrentInitialOpacity = 0.5f;
	float CurrentFadeUpdateInterval = 0.03f;


	FTimerHandle AfterImageSpawnTimer;


	bool bIsSpawning = false;

	UPROPERTY()
	TWeakObjectPtr<UPaperFlipbookComponent> OwnerSpriteComponent;


	virtual void BeginPlay() override;


	UFUNCTION()
	void SpawnAfterImage();

public:
	UFUNCTION(BlueprintCallable, Category = "Afterimage Effect")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Afterimage Effect")
	void StopSpawning();

	UFUNCTION(BlueprintPure, Category = "Afterimage Effect")
	bool IsSpawning() const { return bIsSpawning; }
};
