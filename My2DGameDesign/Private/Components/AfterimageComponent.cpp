#include "Components/AfterimageComponent.h"
#include "GameFramework/Actor.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "TimerManager.h"
#include "Actors/AfterImageActor.h"
#include "MySubsystems/AfterImagePoolSubsystem.h"
#include "Components/DashComponent.h"
#include "DataAssets/HeroDA/HeroFXSettingsDA.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

UAfterimageComponent::UAfterimageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsSpawning = false;
}

void UAfterimageComponent::BeginPlay()
{
	Super::BeginPlay();
	if (FXSettings)
	{
		CurrentAfterImageInterval = FXSettings->AfterImageInterval;
		CurrentAfterImageLifetime = FXSettings->AfterImageLifetime;
		CurrentOpacityParamName = FXSettings->AfterImageOpacityParamName;
		CurrentInitialOpacity = FXSettings->AfterImageInitialOpacity;
		CurrentFadeUpdateInterval = FXSettings->AfterImageFadeUpdateInterval;
	}

	AActor* Owner = GetOwner();
	if (Owner)
	{
		UDashComponent* DashComp = Owner->FindComponentByClass<UDashComponent>();
		if (DashComp)
		{
			DashComp->OnDashStarted_Event.AddDynamic(this, &UAfterimageComponent::StartSpawning);
			DashComp->OnDashEnded_Event.AddDynamic(this, &UAfterimageComponent::StopSpawning);
		}
		else
		{
		}
		OwnerSpriteComponent = Owner->FindComponentByClass<UPaperFlipbookComponent>();
		if (!OwnerSpriteComponent.IsValid())
		{
		}
	}
	
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GameInstance)
	{
		AfterImagePoolSubsystemPtr = GameInstance->GetSubsystem<UAfterImagePoolSubsystem>();
		if (AfterImagePoolSubsystemPtr)
		{
		}
		else
		{
		}
	}

	if (!AfterImagePoolSubsystemPtr)
	{
	}
}

void UAfterimageComponent::StartSpawning()
{
	if (bIsSpawning || !OwnerSpriteComponent.IsValid() || !GetWorld())
	{
		if (!OwnerSpriteComponent.IsValid())
		{
			return;
		}
	}

	bIsSpawning = true;


	SpawnAfterImage();


	GetWorld()->GetTimerManager().SetTimer(
		AfterImageSpawnTimer,
		this,
		&UAfterimageComponent::SpawnAfterImage,
		CurrentAfterImageInterval,
		true
	);
}

void UAfterimageComponent::StopSpawning()
{
	if (!bIsSpawning || !GetWorld())
	{
		return;
	}

	bIsSpawning = false;
	GetWorld()->GetTimerManager().ClearTimer(AfterImageSpawnTimer);
}

void UAfterimageComponent::SpawnAfterImage()
{
	if (!OwnerSpriteComponent.IsValid() || !OwnerSpriteComponent->GetFlipbook() || !GetOwner() || !GetWorld())
	{
		StopSpawning();
		return;
	}
	if (!AfterImagePoolSubsystemPtr)
	{
		StopSpawning();
		return;
	}
	AActor* OwnerActor = GetOwner();
	UPaperFlipbookComponent* SpriteComp = OwnerSpriteComponent.Get();
	FTransform SpriteTransform = SpriteComp->GetComponentTransform();

	AAfterImageActor* GhostActor = AfterImagePoolSubsystemPtr->SpawnFromPool(
		SpriteComp->GetFlipbook(),
		FXSettings->AfterImageBaseMaterial,
		CurrentAfterImageLifetime,
		SpriteTransform,
		CurrentOpacityParamName,
		CurrentInitialOpacity,
		CurrentFadeUpdateInterval
	);

	if (!GhostActor)
	{
	}
}
