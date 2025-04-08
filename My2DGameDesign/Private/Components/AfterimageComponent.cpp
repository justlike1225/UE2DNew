
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
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GameInstance)
	{
		
		       AfterImagePoolSubsystemPtr = GameInstance->GetSubsystem<UAfterImagePoolSubsystem>(); // <--- 获取 Subsystem
		      if (AfterImagePoolSubsystemPtr)
			       {
				           UE_LOG(LogTemp, Log, TEXT("AfterimageComponent: Successfully found UAfterImagePoolSubsystem on GameInstance."));
				      }
		       else
			       {
				            UE_LOG(LogTemp, Error, TEXT("AfterimageComponent: Could not find UAfterImagePoolSubsystem on GameInstance!"));
				       }
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AfterimageComponent: Could not get GameInstance in BeginPlay!"));
	}
	// --- 添加一个最终检查 ---
	if (!AfterImagePoolSubsystemPtr) { UE_LOG(LogTemp, Error, TEXT("AfterimageComponent: BeginPlay finished but AfterImagePoolSubsystemPtr is STILL NULL!")); } // <--- 修改检查
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
	if (!AfterImagePoolSubsystemPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("AfterimageComponent::SpawnAfterImage: Cannot spawn because AfterImagePoolSubsystemPtr is null!")); // <-- Log 修改
		StopSpawning(); // 停止尝试生成
		return;
	}
	// --- 使用对象池获取Actor ---
	if (!AfterImagePoolSubsystemPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("AfterimageComponent::SpawnAfterImage: AfterImagePool is not valid!"));
		StopSpawning(); // 没有池就无法生成
		return;
	}

	AActor* OwnerActor = GetOwner();
	UPaperFlipbookComponent* SpriteComp = OwnerSpriteComponent.Get();
	FTransform SpriteTransform = SpriteComp->GetComponentTransform();

	// --- 添加日志 1 ---
	UE_LOG(LogTemp, Warning, TEXT("SpawnAfterImage: ====> Attempting to spawn via Subsystem: %p. Flipbook: %s. Material: %s"),
		AfterImagePoolSubsystemPtr.Get(),
		SpriteComp ? *SpriteComp->GetName() : TEXT("NULL"),
		AfterImageMaterial ? *AfterImageMaterial->GetName() : TEXT("NULL")
	);
	// 调用对象池的SpawnFromPool函数
	AAfterImageActor* GhostActor = AfterImagePoolSubsystemPtr->SpawnFromPool(
		SpriteComp->GetFlipbook(),
		AfterImageMaterial,         // 材质仍然可以在这个组件上配置
		CurrentAfterImageLifetime,  // 使用从DA加载的生命周期
		SpriteTransform,
		CurrentOpacityParamName,    // 使用从DA加载的参数名
		CurrentInitialOpacity,      // 使用从DA加载的初始透明度
		CurrentFadeUpdateInterval   // 使用从DA加载的更新频率
	);

	// --- 添加日志 2 ---
	UE_LOG(LogTemp, Warning, TEXT("SpawnAfterImage: <==== SpawnFromPool returned Actor: %p (%s)"),
		GhostActor,
		GhostActor ? *GhostActor->GetName() : TEXT("NULL")
	);
	if (!GhostActor)
	{
		// SpawnFromPool 返回 nullptr，意味着池已满或出错
		UE_LOG(LogTemp, Warning, TEXT("AfterimageComponent: Failed to get an actor from AfterImagePoolSubsystem...")); // <-- Log 修改
		
	}
	// 注意：不再需要手动调用 GhostActor->Initialize 了，因为 SpawnFromPool 内部会调用 Activate
}