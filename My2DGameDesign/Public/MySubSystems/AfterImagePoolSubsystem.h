// My2DGameDesign/Public/Subsystems/AfterImagePoolSubsystem.h
#pragma once
#include  "Subsystems/GameInstanceSubsystem.h"
#include "CoreMinimal.h"
#include "Templates/SubclassOf.h" // 需要 TSubclassOf
#include "AfterImagePoolSubsystem.generated.h"

// 前向声明
class AAfterImageActor;
class UPaperFlipbook;
class UMaterialInterface;

/**
 * 管理残影 Actor (AAfterImageActor) 对象池的 GameInstance 子系统。
 * 提供全局访问的对象池服务。
 */
UCLASS()
class MY2DGAMEDESIGN_API UAfterImagePoolSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // --- UGameInstanceSubsystem 接口 ---
    virtual void Initialize(FSubsystemCollectionBase& Collection) override; // 替代 BeginPlay
    virtual void Deinitialize() override;                                  // 替代 EndPlay

    // --- 公开的对象池接口函数 (从原 Component 移来) ---

    /**
     * @brief 从对象池获取或创建一个残影Actor实例并激活它。
     * @param FlipbookToCopy (同 AAfterImageActor::Activate)
     * @param MaterialToUse (同 AAfterImageActor::Activate)
     * @param LifeTime (同 AAfterImageActor::Activate)
     * @param SpriteTransform (同 AAfterImageActor::Activate)
     * @param OpacityParamName (同 AAfterImageActor::Activate)
     * @param InitialOpacity (同 AAfterImageActor::Activate)
     * @param FadeUpdateInterval (同 AAfterImageActor::Activate)
     * @return 返回激活的 AAfterImageActor 指针，如果池已满且无法创建新实例，则返回 nullptr。
     */
    UFUNCTION(BlueprintCallable, Category = "Object Pool") // 保持 BlueprintCallable 如果需要在蓝图中使用
    AAfterImageActor* SpawnFromPool(
        UPaperFlipbook* FlipbookToCopy,
        UMaterialInterface* MaterialToUse,
        float LifeTime,
        const FTransform& SpriteTransform,
        FName OpacityParamName,
        float InitialOpacity,
        float FadeUpdateInterval
    );

    /**
     * @brief 将一个不再使用的 AAfterImageActor 实例归还到对象池中。
     * 通常由 AAfterImageActor 自身调用。
     * @param ActorToReturn 要归还的Actor实例。
     */
    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    void ReturnToPool(AAfterImageActor* ActorToReturn);


protected:
    // --- 配置变量 (从原 Component 移来) ---
    // 注意：这些变量现在如何配置？
    // 选项1: 直接在 C++ 初始化 (如下面的硬编码)
    // 选项2: UPROPERTY(Config) 并在 .ini 文件中配置
    // 选项3: 引用一个 DataAsset 来读取配置
    // 我们先用硬编码，你可以后续修改

    /** @brief 要池化的Actor类。 【重要】这里暂时硬编码，你需要替换成你的实际类！*/
    TSubclassOf<AAfterImageActor> PooledActorClass = nullptr; // 下面 Initialize 里会设置

    /** @brief 对象池的初始大小。会在 Initialize 时预先生成这么多Actor。*/
    int32 InitialPoolSize = 15;

    /** @brief 对象池的最大大小。如果为0或负数，表示不限制（不推荐）。*/
    int32 MaxPoolSize = 30;

    /** @brief 如果池中没有可用对象，是否允许动态生成新的Actor（直到达到MaxPoolSize）？*/
    bool bAllowPoolGrowth = true;


private:
    // --- 内部状态变量 (从原 Component 移来) ---

    /** @brief 存储所有当前未被使用的Actor实例。*/
    UPROPERTY(Transient) // 运行时变量，不需要保存
    TArray<AAfterImageActor*> InactivePool;

    /** @brief 存储所有由该池管理（包括激活和非激活）的Actor实例，方便统一销毁。*/
    UPROPERTY(Transient)
    TArray<AAfterImageActor*> AllManagedActors;

    /** @brief 记录当前池中的总Actor数量 (包括激活和非激活) */
    int32 CurrentTotalPooledActors = 0;

    // --- 内部辅助函数 (从原 Component 移来) ---

    /**
     * @brief 尝试扩展对象池，生成一个新的Actor实例。
     * @return 返回新生成的Actor指针，如果已达到MaxPoolSize或生成失败，则返回 nullptr。
     */
    AAfterImageActor* TryGrowPool();

    /** 预生成池中 Actor 的逻辑 */
    void PrewarmPool();

     /** 清理所有管理的 Actor */
     void CleanupPool();
};