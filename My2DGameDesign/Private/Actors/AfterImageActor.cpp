// My2DGameDesign/Private/AfterImageActor.cpp

#include "Actors/AfterImageActor.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "MySubSystems/AfterImagePoolSubsystem.h"

AAfterImageActor::AAfterImageActor()
{
	PrimaryActorTick.bCanEverTick = false;

	AfterImageSprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("AfterImageSprite"));
	RootComponent = AfterImageSprite;

	AfterImageSprite->SetCollisionProfileName(TEXT("NoCollision"));
	AfterImageSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AfterImageSprite->Stop();
	AfterImageSprite->SetLooping(false);
	AfterImageSprite->SetVisibility(false);

	AActor::SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	AActor::SetActorTickEnabled(false);
}

void AAfterImageActor::BeginPlay()
{
	Super::BeginPlay();
	if (!bIsActive)
	{
		ResetActor();
	}
}

void AAfterImageActor::Activate(
	UPaperFlipbook* FlipbookToCopy,
	UMaterialInterface* MaterialToUse,
	float LifeTime,
	const FTransform& SpriteTransform,
	FName InOpacityParamName,
	float InInitialOpacity,
	float InFadeUpdateInterval,
	UAfterImagePoolSubsystem* InOwningSubsystem)
{
	if (bIsActive || !FlipbookToCopy || LifeTime <= 0.0f || !InOwningSubsystem)
	{
		if (bIsActive)
		{
			Deactivate();
		}
		return;
	}

	OwningPoolSubsystemPtr = InOwningSubsystem;
	ActorLifeTime = LifeTime;
	OpacityParameterName = InOpacityParamName;
	CurrentInitialOpacity = InInitialOpacity;
	CreationTime = UGameplayStatics::GetTimeSeconds(GetWorld());

	SetActorTransform(SpriteTransform);

	if (IsValid(AfterImageSprite))
	{
		AfterImageSprite->SetFlipbook(FlipbookToCopy);
	}

	if (MaterialToUse && !OpacityParameterName.IsNone() && IsValid(AfterImageSprite))
	{
		MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(MaterialToUse, this);
		if (MaterialInstanceDynamic)
		{
			AfterImageSprite->SetMaterial(0, MaterialInstanceDynamic);
			MaterialInstanceDynamic->SetScalarParameterValue(OpacityParameterName, CurrentInitialOpacity);
			GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &AAfterImageActor::UpdateFade, InFadeUpdateInterval,
			                                true);
		}
	}

	bIsActive = true;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);
	if (IsValid(AfterImageSprite))
	{
		AfterImageSprite->SetVisibility(true);
		AfterImageSprite->PlayFromStart();
	}

	GetWorldTimerManager().SetTimer(LifetimeTimerHandle, this, &AAfterImageActor::OnLifetimeExpired, ActorLifeTime,
	                                false);
}

void AAfterImageActor::Deactivate()
{
	if (!bIsActive)
	{
		return;
	}

	TWeakObjectPtr<UAfterImagePoolSubsystem> PoolPtr = OwningPoolSubsystemPtr;

	ResetActor();

	bool bReturnedToPool = false;
	if (PoolPtr.IsValid())
	{
		PoolPtr->ReturnToPool(this);
		bReturnedToPool = true;
	}
	else
	{
		Destroy();
	}
}

void AAfterImageActor::ResetActor()
{
	bIsActive = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	if (IsValid(AfterImageSprite))
	{
		AfterImageSprite->SetVisibility(false);
		AfterImageSprite->Stop();
		AfterImageSprite->SetFlipbook(nullptr);
	}

	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(FadeTimerHandle);
		GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
	}

	MaterialInstanceDynamic = nullptr;

	OwningPoolSubsystemPtr = nullptr;
	ActorLifeTime = 0.0f;
	CreationTime = 0.0f;
}


void AAfterImageActor::UpdateFade()
{
	if (!bIsActive || !MaterialInstanceDynamic)
	{
		GetWorldTimerManager().ClearTimer(FadeTimerHandle);
		return;
	}

	float ElapsedTime = UGameplayStatics::GetTimeSeconds(GetWorld()) - CreationTime;
	float LifeFraction = FMath::Clamp(ElapsedTime / ActorLifeTime, 0.0f, 1.0f);
	float CurrentOpacity = FMath::Lerp(CurrentInitialOpacity, 0.0f, LifeFraction);
	MaterialInstanceDynamic->SetScalarParameterValue(OpacityParameterName, CurrentOpacity);

	if (LifeFraction >= 1.0f)
	{
		GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	}
}

void AAfterImageActor::OnLifetimeExpired()
{
	Deactivate();
}

void AAfterImageActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetActor();
	Super::EndPlay(EndPlayReason);
}
