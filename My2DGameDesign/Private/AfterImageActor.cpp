// My2DGameDesign/Private/AfterImageActor.cpp

#include "AfterImageActor.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AfterImagePoolComponent.h" 
AAfterImageActor::AAfterImageActor()
{
	PrimaryActorTick.bCanEverTick = false; // 仍然不需要Tick

	AfterImageSprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("AfterImageSprite"));
	RootComponent = AfterImageSprite;

	AfterImageSprite->SetCollisionProfileName(TEXT("NoCollision"));
	AfterImageSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AfterImageSprite->Stop();
	AfterImageSprite->SetLooping(false);
	AfterImageSprite->SetVisibility(false); // 默认隐藏

	// 设置Actor默认不激活，不参与碰撞等
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false); // 确保Tick也禁用
}

void AAfterImageActor::BeginPlay()
{
	Super::BeginPlay();
	// 初始状态设置为非激活
	if (!bIsActive)
	{
		ResetActor(); // 确保初始状态干净
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
	UAfterImagePoolComponent* InOwningPool) // <--- 接收对象池指针
{
	
	// 在函数开头添加日志
	UE_LOG(LogTemp, Log, TEXT("AAfterImageActor '%s'::Activate received InOwningPool pointer: %p"), *GetName(), InOwningPool);

	if (bIsActive || !FlipbookToCopy || LifeTime <= 0.0f || !InOwningPool)
	{
		UE_LOG(LogTemp, Warning, TEXT("AfterImageActor::Activate called with invalid parameters or while already active."));
		// 如果尝试激活一个已经激活的或者参数无效，可能需要强制Deactivate或直接返回
        if(bIsActive) Deactivate(); // 如果已激活，先尝试反激活
		return;
	}

	UE_LOG(LogTemp, Verbose, TEXT("Activating AfterImageActor: %s"), *GetName());

	// --- 存储状态 ---
	ActorLifeTime = LifeTime;
	OpacityParameterName = InOpacityParamName;
	CurrentInitialOpacity = InInitialOpacity;
	OwningPoolPtr = InOwningPool; // <--- 存储对象池引用
	CreationTime = UGameplayStatics::GetTimeSeconds(GetWorld()); // 记录激活时间

	// --- 设置外观和位置 ---
	AfterImageSprite->SetFlipbook(FlipbookToCopy);
	SetActorTransform(SpriteTransform); // 使用 SetActorTransform

	// --- 创建或重用并设置动态材质 ---
	if (MaterialToUse && !OpacityParameterName.IsNone())
	{
		if (!MaterialInstanceDynamic || MaterialInstanceDynamic->Parent != MaterialToUse)
		{
			// 如果MID不存在，或者父材质不对，则创建新的
			MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(MaterialToUse, this);
			AfterImageSprite->SetMaterial(0, MaterialInstanceDynamic);
		}
        else
        {
            // MID 已存在且父材质正确，可以直接用
            AfterImageSprite->SetMaterial(0, MaterialInstanceDynamic); // 确保应用了
        }

		if (MaterialInstanceDynamic)
		{
			MaterialInstanceDynamic->SetScalarParameterValue(OpacityParameterName, CurrentInitialOpacity);
			// 启动淡出更新定时器
			GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &AAfterImageActor::UpdateFade, InFadeUpdateInterval, true);
		}
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("AfterImageActor '%s': Failed to create or reuse Dynamic Material Instance."), *GetName());
             // 没材质效果，但至少让它显示出来
        }
	}
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("AfterImageActor '%s': Invalid Material or Opacity Parameter Name provided."), *GetName());
    }


	// --- 设置激活状态 ---
	bIsActive = true;
	SetActorHiddenInGame(false); // 显示Actor
	SetActorEnableCollision(false); // 残影通常不需要碰撞
	AfterImageSprite->SetVisibility(true); // 确保Sprite可见
	AfterImageSprite->PlayFromStart(); // 播放动画 (如果需要的话)

	// --- 启动生命周期定时器 ---
	// 不再使用 SetLifeSpan，而是用定时器在LifeTime后调用OnLifetimeExpired
	GetWorldTimerManager().SetTimer(LifetimeTimerHandle, this, &AAfterImageActor::OnLifetimeExpired, ActorLifeTime, false);
}

void AAfterImageActor::Deactivate()
{
	if (!bIsActive) return;

	UE_LOG(LogTemp, Verbose, TEXT("Deactivating AfterImageActor: %s"), *GetName());

	// --- 1. 先尝试通知对象池回收 ---
	// 打印指针值用于调试 (可以保留或移除)
	UE_LOG(LogTemp, Log, TEXT("AAfterImageActor '%s'::Deactivate - Checking OwningPoolPtr value just before use: %p"), *GetName(), OwningPoolPtr.Get());

	bool bReturnedToPool = false; // 标记是否成功返回池中
	if (OwningPoolPtr.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("AAfterImageActor '%s'::Deactivate - OwningPoolPtr check passed. Attempting to return to pool."), *GetName());
		OwningPoolPtr->ReturnToPool(this);
		bReturnedToPool = true; // 标记成功返回
	}
	else
	{
		// 如果此时指针就已经是 null (理论上不应该发生，除非 Activate 就没成功)
		UE_LOG(LogTemp, Warning, TEXT("AAfterImageActor '%s' could not return to pool (OwningPoolPtr was null BEFORE ResetActor)."), *GetName());
	}

	// --- 2. 然后再重置 Actor 状态 ---
	// 注意：ResetActor内部会设置 OwningPoolPtr = nullptr; 这没关系，因为我们已经用过它了
	ResetActor();

	// --- 3. 如果未能成功返回对象池，则销毁自己作为备用方案 ---
	if (!bReturnedToPool)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAfterImageActor '%s' was not returned to pool. Destroying self."), *GetName());
		// 注意：这里确保只有在无法返回池时才销毁，而不是每次都销毁
		Destroy();
	}
	// 如果成功返回了池，则不需要 Destroy()
}

// 重置Actor状态，用于初始化和Deactivate
void AAfterImageActor::ResetActor()
{
    bIsActive = false;
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);
    AfterImageSprite->SetVisibility(false);
    AfterImageSprite->Stop(); // 停止动画
	AfterImageSprite->SetFlipbook(nullptr); // 清空Flipbook引用

	// 清理所有定时器
    if(GetWorld()) // 检查GetWorld是否有效
    {
	    GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	    GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
    }

	// 重置材质状态 (可选，看是否需要恢复默认)
	// if (MaterialInstanceDynamic) { MaterialInstanceDynamic->SetScalarParameterValue(OpacityParameterName, CurrentInitialOpacity); }

    // 从场景中断开附加？通常不需要，隐藏即可。
    // SetActorLocation(FVector(0,0,-10000)); // 可以移到远处

	OwningPoolPtr = nullptr; // 清除对池的引用
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

    // 淡出完成后不再需要做什么，等待LifetimeTimer触发Deactivate
    if (LifeFraction >= 1.0f) {
         GetWorldTimerManager().ClearTimer(FadeTimerHandle); // 可以停止更新了
    }
}

void AAfterImageActor::OnLifetimeExpired()
{
    // 生命周期自然结束，触发反激活
	Deactivate();
}

void AAfterImageActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 确保Actor被意外销毁时（例如关卡切换），定时器被清理
    ResetActor(); // 调用ResetActor可以清理定时器
	Super::EndPlay(EndPlayReason);
}