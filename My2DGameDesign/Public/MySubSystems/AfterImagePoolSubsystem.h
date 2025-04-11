#pragma once
#include  "Subsystems/GameInstanceSubsystem.h"
#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "AfterImagePoolSubsystem.generated.h"

class AAfterImageActor;
class UPaperFlipbook;
class UMaterialInterface;

UCLASS()
class MY2DGAMEDESIGN_API UAfterImagePoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	AAfterImageActor* SpawnFromPool(
		UPaperFlipbook* FlipbookToCopy,
		UMaterialInterface* MaterialToUse,
		float LifeTime,
		const FTransform& SpriteTransform,
		FName OpacityParamName,
		float InitialOpacity,
		float FadeUpdateInterval
	);

	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnToPool(AAfterImageActor* ActorToReturn);

protected:
	TSubclassOf<AAfterImageActor> PooledActorClass = nullptr;
	int32 InitialPoolSize = 15;
	int32 MaxPoolSize = 30;
	bool bAllowPoolGrowth = true;

	UPROPERTY(Transient)
	TArray<AAfterImageActor*> InactivePool;

	UPROPERTY(Transient)
	TArray<AAfterImageActor*> AllManagedActors;

	int32 CurrentTotalPooledActors = 0;

	AAfterImageActor* TryGrowPool();
	void PrewarmPool();
	void CleanupPool();
};
