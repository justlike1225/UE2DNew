// My2DGameDesign/Private/AfterImageActor.cpp

#include "Actors/AfterImageActor.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "MySubSystems/AfterImagePoolSubsystem.h" // 引入对象池组件
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
    UAfterImagePoolSubsystem* InOwningSubsystem)
{
    // --- 日志：函数入口和输入参数 ---
    UE_LOG(LogTemp, Error, TEXT("Activate %s: Entered. FlipbookArg=%s, MaterialArg=%s, LifetimeArg=%.2f, TransformArgLoc=%s"),
        *GetName(),
        FlipbookToCopy ? *FlipbookToCopy->GetName() : TEXT("NULL"),
        MaterialToUse ? *MaterialToUse->GetName() : TEXT("NULL"),
        LifeTime,
        *SpriteTransform.GetLocation().ToString() // 打印传入的 Transform 位置
    );

    if (bIsActive || !FlipbookToCopy || LifeTime <= 0.0f || !InOwningSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Activate %s: Aborted due to invalid params or already active."), *GetName());
        if(bIsActive) Deactivate();
        return;
    }

    // --- 日志：检查核心组件指针 ---
    UE_LOG(LogTemp, Error, TEXT("Activate %s: Checking pointers BEFORE Set... AfterImageSprite=%p (%s)"),
        *GetName(),
        AfterImageSprite,
        IsValid(AfterImageSprite) ? TEXT("Valid") : TEXT("INVALID!") // 使用 IsValid() 检查 UObject 指针
    );

    OwningPoolSubsystemPtr = InOwningSubsystem;
    ActorLifeTime = LifeTime;
    OpacityParameterName = InOpacityParamName;
    CurrentInitialOpacity = InInitialOpacity;
    CreationTime = UGameplayStatics::GetTimeSeconds(GetWorld());

    // --- 设置 Transform 并立刻检查 ---
    SetActorTransform(SpriteTransform); // 尝试设置 Transform
    FVector CurrentLocation = GetActorLocation(); // 立刻获取当前位置
    FQuat CurrentRotation = GetActorRotation().Quaternion(); // 立刻获取当前旋转
    UE_LOG(LogTemp, Error, TEXT("Activate %s: SetActorTransform called. TargetLoc=%s, ResultLoc=%s. TargetRot=%s, ResultRot=%s"),
        *GetName(),
        *SpriteTransform.GetLocation().ToString(), // 目标位置
        *CurrentLocation.ToString(),              // 实际位置
        *SpriteTransform.GetRotation().ToString(),  // 目标旋转
        *CurrentRotation.ToString()                 // 实际旋转
        );

    // --- 设置 Flipbook 并立刻检查 ---
    if (IsValid(AfterImageSprite)) // 再次检查确保 Sprite 有效
    {
        AfterImageSprite->SetFlipbook(FlipbookToCopy); // 尝试设置 Flipbook
        UPaperFlipbook* ResultFlipbook = AfterImageSprite->GetFlipbook(); // 立刻获取结果
        UE_LOG(LogTemp, Error, TEXT("Activate %s: SetFlipbook called. Target=%s, Result=%s"),
            *GetName(),
            FlipbookToCopy ? *FlipbookToCopy->GetName() : TEXT("NULL"),
            ResultFlipbook ? *ResultFlipbook->GetName() : TEXT("NULL")
        );
    }

    // --- 创建/设置材质并立刻检查 ---
    if (MaterialToUse && !OpacityParameterName.IsNone() && IsValid(AfterImageSprite))
    {
        // 创建动态材质实例 (Create potentially returns nullptr if MaterialToUse is invalid)
        MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(MaterialToUse, this);
        UE_LOG(LogTemp, Error, TEXT("Activate %s: MID Created: %p. Parent Material: %s"),
             *GetName(),
             MaterialInstanceDynamic,
             MaterialToUse ? *MaterialToUse->GetName() : TEXT("NULL")
        );

        if (MaterialInstanceDynamic) // 确保 MID 创建成功
        {
            AfterImageSprite->SetMaterial(0, MaterialInstanceDynamic); // 尝试设置材质
            UMaterialInterface* ResultMaterial = AfterImageSprite->GetMaterial(0); // 立刻获取结果
             UE_LOG(LogTemp, Error, TEXT("Activate %s: SetMaterial called. Target=%s, Result=%s"),
                 *GetName(),
                 MaterialInstanceDynamic ? *MaterialInstanceDynamic->GetName() : TEXT("NULL"),
                 ResultMaterial ? *ResultMaterial->GetName() : TEXT("NULL")
            );

            MaterialInstanceDynamic->SetScalarParameterValue(OpacityParameterName, CurrentInitialOpacity);
            GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &AAfterImageActor::UpdateFade, InFadeUpdateInterval, true);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Activate %s: Failed to create MaterialInstanceDynamic!"), *GetName());
        }
    }
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("Activate %s: Skipped Material setup (MaterialToUse null, OpacityName none, or Sprite invalid)."), *GetName());
    }


    // --- 设置可见性和激活状态 ---
    bIsActive = true;
    SetActorHiddenInGame(false);
    SetActorEnableCollision(false);
    if (IsValid(AfterImageSprite))
    {
        AfterImageSprite->SetVisibility(true);
        AfterImageSprite->PlayFromStart();
    }

    // --- 启动生命周期定时器 ---
    GetWorldTimerManager().SetTimer(LifetimeTimerHandle, this, &AAfterImageActor::OnLifetimeExpired, ActorLifeTime, false);

    // --- 日志：函数结束时的状态 ---
    UE_LOG(LogTemp, Error, TEXT("Activate %s: Finished. SpriteVisible=%d, ActorHidden=%d"),
        *GetName(),
        IsValid(AfterImageSprite) ? AfterImageSprite->IsVisible() : -1, // 加个 IsValid 检查
        IsHidden()
    );
}

void AAfterImageActor::Deactivate()
{
    // 1. 防止重复进入 Deactivate 逻辑
    if (!bIsActive)
    {
        return;
    }

    UE_LOG(LogTemp, Verbose, TEXT("Deactivating AfterImageActor: %s"), *GetName());

    // 2. 【关键】在重置状态之前，先临时保存对象池的指针
    // 因为 ResetActor 内部会把 OwningPoolSubsystemPtr 置空
    TWeakObjectPtr<UAfterImagePoolSubsystem> PoolPtr = OwningPoolSubsystemPtr;

    // 3. 【关键】先调用 ResetActor 来重置所有状态
    // 这会设置 bIsActive = false, 清理定时器, 隐藏 Actor 和 Sprite 等
    ResetActor();

    // 4. 尝试使用之前保存的指针将自身归还到对象池
    bool bReturnedToPool = false;
    if (PoolPtr.IsValid()) // 使用临时保存的指针检查有效性
    {
        UE_LOG(LogTemp, Log, TEXT("AAfterImageActor '%s'::Deactivate - Attempting to return to pool."), *GetName());
        PoolPtr->ReturnToPool(this); // 使用临时保存的指针调用 ReturnToPool
        bReturnedToPool = true;
    }
    else
    {
        // 如果临时指针也无效（理论上不应该，除非 Activate 就没成功传入），记录警告
        UE_LOG(LogTemp, Warning, TEXT("AAfterImageActor '%s' could not return to pool (Stored PoolPtr was invalid?)."), *GetName());
    }

    // 5. 如果因为某些原因（比如 PoolPtr 无效）导致归还失败，则销毁 Actor 作为后备措施
    if (!bReturnedToPool)
    {
        UE_LOG(LogTemp, Warning, TEXT("AAfterImageActor '%s' was not returned to pool. Destroying self."), *GetName());
        Destroy();
    }
    // 注意：如果归还成功，Actor 实例会保留在对象池中，不会被销毁。
    // 同时因为 ResetActor 已经将 bIsActive 设为 false，所以这个函数不会再次进入。
}

// --- 你现有的 ResetActor 函数 (确保它包含了设置 bIsActive = false) ---
void AAfterImageActor::ResetActor()
{
    bIsActive = false; // <--- 确保这一行存在且有效！
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);

    if (IsValid(AfterImageSprite)) // 检查指针有效性
    {
        AfterImageSprite->SetVisibility(false);
        AfterImageSprite->Stop(); // 停止动画
        AfterImageSprite->SetFlipbook(nullptr); // 清空Flipbook引用
        // 可选：也可以考虑重置材质为默认，但这通常不是必须的
        // AfterImageSprite->SetMaterial(0, nullptr);
    }


    // 清理所有定时器
    if(GetWorld()) // 检查GetWorld是否有效
    {
        GetWorldTimerManager().ClearTimer(FadeTimerHandle);
        GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
    }

    // 重置材质状态 (可选，看是否需要恢复默认)
    // 不需要手动管理 MaterialInstanceDynamic 的销毁，下次 Activate 会重新创建或覆盖
    MaterialInstanceDynamic = nullptr; // 可以选择置空引用

    OwningPoolSubsystemPtr = nullptr; // 清除对池的引用 (这没问题，因为我们在 Deactivate 里临时保存了)
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