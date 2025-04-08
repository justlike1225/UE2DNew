
#include "Components/AfterimageComponent.h"
#include "GameFramework/Actor.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "TimerManager.h"
#include "Actors/AfterImageActor.h" 
#include "Components/AfterImagePoolComponent.h"
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
	if(FXSettings)
	{
		CurrentAfterImageInterval = FXSettings->AfterImageInterval;
		CurrentAfterImageLifetime = FXSettings->AfterImageLifetime;
		CurrentOpacityParamName = FXSettings->AfterImageOpacityParamName;
		CurrentInitialOpacity = FXSettings->AfterImageInitialOpacity;
		CurrentFadeUpdateInterval = FXSettings->AfterImageFadeUpdateInterval;
		UE_LOG(LogTemp, Log, TEXT("AfterimageComponent: Loaded settings from DA %s"), *FXSettings->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AfterimageComponent: No FXSettings Data Asset assigned. Using default values."));
	}
    // 获取拥有者Actor
    AActor* Owner = GetOwner();
    if(Owner)
    {
    	UDashComponent* DashComp = Owner->FindComponentByClass<UDashComponent>();
    	if (DashComp)
    	{
    		// 绑定到 DashComponent 的事件
    		DashComp->OnDashStarted_Event.AddDynamic(this, &UAfterimageComponent::StartSpawning);
    		DashComp->OnDashEnded_Event.AddDynamic(this, &UAfterimageComponent::StopSpawning);
    		UE_LOG(LogTemp, Log, TEXT("AfterimageComponent bound to DashComponent events."));
    	}
    	else
    	{
    		UE_LOG(LogTemp, Warning, TEXT("AfterimageComponent: Could not find DashComponent on owner '%s' to bind events."), *Owner->GetName());
    	}
        // 查找拥有者Actor的UPaperFlipbookComponent组件
        OwnerSpriteComponent = Owner->FindComponentByClass<UPaperFlipbookComponent>();
        if(!OwnerSpriteComponent.IsValid())
        {
             UE_LOG(LogTemp, Warning, TEXT("AfterimageComponent: Owner Actor '%s' does not have a UPaperFlipbookComponent."), *Owner->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AfterimageComponent has no Owner Actor!"));
    }
	// --- 查找对象池组件 (修改后的逻辑) ---
	UWorld* World = GetWorld();
	if (World)
	{
		// 获取当前关卡的GameMode实例
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(World); // 或者 World->GetAuthGameMode();
		if (GameMode)
		{
			// 从GameMode实例上查找我们的PoolComponent
			AfterImagePool = GameMode->FindComponentByClass<UAfterImagePoolComponent>();
			if (AfterImagePool)
			{
				UE_LOG(LogTemp, Log, TEXT("AfterimageComponent: Successfully found UAfterImagePoolComponent on GameMode '%s'."), *GameMode->GetName());
			}
			else
			{
				// GameMode找到了，但上面没有PoolComponent！
				UE_LOG(LogTemp, Error, TEXT("AfterimageComponent: Found GameMode '%s', but UAfterImagePoolComponent was NOT attached to it!"), *GameMode->GetName());
			}
		}
		else
		{
			// 无法获取GameMode实例
			UE_LOG(LogTemp, Error, TEXT("AfterimageComponent: Could not get GameMode instance from World!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AfterimageComponent: Could not get World in BeginPlay!"));
	}

	// --- 添加一个最终检查 ---
	if (!AfterImagePool)
	{
		UE_LOG(LogTemp, Error, TEXT("AfterimageComponent: BeginPlay finished but AfterImagePool is STILL NULL!"));
	}
}


void UAfterimageComponent::StartSpawning()
{
	if (bIsSpawning || !AfterImageClass || !OwnerSpriteComponent.IsValid() || !GetWorld())
	{
        if(!OwnerSpriteComponent.IsValid()) UE_LOG(LogTemp, Warning, TEXT("AfterimageComponent::StartSpawning: Cannot start, Owner Sprite Component is invalid or not found."));
		return; 
	}

    UE_LOG(LogTemp, Log, TEXT("AfterimageComponent: Starting Spawning"));
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

    UE_LOG(LogTemp, Log, TEXT("AfterimageComponent: Stopping Spawning"));
    bIsSpawning = false;
	GetWorld()->GetTimerManager().ClearTimer(AfterImageSpawnTimer);
}

void UAfterimageComponent::SpawnAfterImage()
{
	// 检查基本条件 (Owner, Sprite, Flipbook等)
	if (!OwnerSpriteComponent.IsValid() || !OwnerSpriteComponent->GetFlipbook() || !GetOwner() || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("AfterimageComponent::SpawnAfterImage: Aborting spawn due to invalid base state (Sprite, Flipbook, Owner, or World)."));
		StopSpawning();
		return;
	}
	// --- 在调用前再次检查 AfterImagePool 是否有效 ---
	if (!AfterImagePool)
	{
		UE_LOG(LogTemp, Error, TEXT("AfterimageComponent::SpawnAfterImage: Cannot spawn because AfterImagePool is null!"));
		StopSpawning(); // 停止尝试生成
		return;
	}
	// --- 使用对象池获取Actor ---
	if (!AfterImagePool)
	{
		UE_LOG(LogTemp, Error, TEXT("AfterimageComponent::SpawnAfterImage: AfterImagePool is not valid!"));
		StopSpawning(); // 没有池就无法生成
		return;
	}

	AActor* OwnerActor = GetOwner();
	UPaperFlipbookComponent* SpriteComp = OwnerSpriteComponent.Get();
	FTransform SpriteTransform = SpriteComp->GetComponentTransform();

	// 调用对象池的SpawnFromPool函数
	AAfterImageActor* GhostActor = AfterImagePool->SpawnFromPool(
		SpriteComp->GetFlipbook(),
		AfterImageMaterial,         // 材质仍然可以在这个组件上配置
		CurrentAfterImageLifetime,  // 使用从DA加载的生命周期
		SpriteTransform,
		CurrentOpacityParamName,    // 使用从DA加载的参数名
		CurrentInitialOpacity,      // 使用从DA加载的初始透明度
		CurrentFadeUpdateInterval   // 使用从DA加载的更新频率
	);

	if (!GhostActor)
	{
		// SpawnFromPool 返回 nullptr，意味着池已满或出错
		UE_LOG(LogTemp, Warning, TEXT("AfterimageComponent: Failed to get an actor from AfterImagePool. Pool might be full or encountered an error."));
		
	}
	// 注意：不再需要手动调用 GhostActor->Initialize 了，因为 SpawnFromPool 内部会调用 Activate
}