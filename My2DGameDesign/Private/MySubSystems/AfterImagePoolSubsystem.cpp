// My2DGameDesign/Private/Subsystems/AfterImagePoolSubsystem.cpp
#include "MySubSystems/AfterImagePoolSubsystem.h"
#include "Actors/AfterImageActor.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"

void UAfterImagePoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PooledActorClass = AAfterImageActor::StaticClass();
	if (!PooledActorClass)
	{
		return;
	}

	FTimerHandle TempHandle;
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UAfterImagePoolSubsystem::PrewarmPool);
}

void UAfterImagePoolSubsystem::Deinitialize()
{
	CleanupPool();
	Super::Deinitialize();
}

void UAfterImagePoolSubsystem::PrewarmPool()
{
	if (!GetWorld() || !PooledActorClass)
	{
		return;
	}

	for (int32 i = 0; i < InitialPoolSize; ++i)
	{
		AAfterImageActor* NewActor = TryGrowPool();
		if (NewActor)
		{
			InactivePool.Add(NewActor);
		}
		else
		{
			break;
		}
	}
}

void UAfterImagePoolSubsystem::CleanupPool()
{
	for (int32 i = AllManagedActors.Num() - 1; i >= 0; --i)
	{
		AAfterImageActor* Actor = AllManagedActors[i];
		if (Actor && !Actor->IsPendingKillPending())
		{
			Actor->SetLifeSpan(0.1f);
		}
	}
	AllManagedActors.Empty();
	InactivePool.Empty();
	CurrentTotalPooledActors = 0;
}

AAfterImageActor* UAfterImagePoolSubsystem::SpawnFromPool(
	UPaperFlipbook* FlipbookToCopy,
	UMaterialInterface* MaterialToUse,
	float LifeTime,
	const FTransform& SpriteTransform,
	FName OpacityParamName,
	float InitialOpacity,
	float FadeUpdateInterval)
{
	AAfterImageActor* ActorToActivate = nullptr;

	if (InactivePool.Num() > 0)
	{
		ActorToActivate = InactivePool.Pop();
	}
	else if (bAllowPoolGrowth)
	{
		ActorToActivate = TryGrowPool();
		if (ActorToActivate)
		{
		}
		else
		{
		}
	}
	

	if (ActorToActivate)
	{
		ActorToActivate->Activate(
			FlipbookToCopy,
			MaterialToUse,
			LifeTime,
			SpriteTransform,
			OpacityParamName,
			InitialOpacity,
			FadeUpdateInterval,
			this
		);
		return ActorToActivate;
	}

	return nullptr;
}

void UAfterImagePoolSubsystem::ReturnToPool(AAfterImageActor* ActorToReturn)
{
	if (!ActorToReturn)
	{
		return;
	}

	if (ActorToReturn->IsActive())
	{
	}

	if (!InactivePool.Contains(ActorToReturn))
	{
		InactivePool.Add(ActorToReturn);
	}
	
}

AAfterImageActor* UAfterImagePoolSubsystem::TryGrowPool()
{
	if (MaxPoolSize > 0 && CurrentTotalPooledActors >= MaxPoolSize)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World || !PooledActorClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAfterImageActor* NewActor = World->SpawnActor<AAfterImageActor>(PooledActorClass, FVector::ZeroVector,
	                                                                 FRotator::ZeroRotator, SpawnParams);

	if (NewActor)
	{
		CurrentTotalPooledActors++;
		AllManagedActors.Add(NewActor);
		NewActor->Deactivate();
		return NewActor;
	}
	return nullptr;
}
