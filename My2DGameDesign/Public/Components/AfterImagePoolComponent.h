// My2DGameDesign/Public/Components/AfterImagePoolComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AfterImagePoolComponent.generated.h"

class AAfterImageActor;
class UPaperFlipbook;
class UMaterialInterface;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MY2DGAMEDESIGN_API UAfterImagePoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAfterImagePoolComponent();

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

	/**
	 * @brief 将一个不再使用的 AAfterImageActor 实例归还到对象池中。
	 * 通常由 AAfterImageActor 自身调用。
	 * @param ActorToReturn 要归还的Actor实例。
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnToPool(AAfterImageActor* ActorToReturn);


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** @brief 要池化的Actor类。*/
	UPROPERTY(EditDefaultsOnly, Category = "Object Pool")
	TSubclassOf<AAfterImageActor> PooledActorClass;

	/** @brief 对象池的初始大小。会在BeginPlay时预先生成这么多Actor。*/
	UPROPERTY(EditDefaultsOnly, Category = "Object Pool", meta = (ClampMin = "0"))
	int32 InitialPoolSize = 15;

    /** @brief 对象池的最大大小。如果为0或负数，表示不限制（不推荐）。*/
	UPROPERTY(EditDefaultsOnly, Category = "Object Pool", meta = (ClampMin = "0"))
	int32 MaxPoolSize = 30;

    /** @brief 如果池中没有可用对象，是否允许动态生成新的Actor（直到达到MaxPoolSize）？*/
	UPROPERTY(EditDefaultsOnly, Category = "Object Pool")
	bool bAllowPoolGrowth = true;


private:
	/** @brief 存储所有当前未被使用的Actor实例。*/
	UPROPERTY()
	TArray<AAfterImageActor*> InactivePool;

	/** @brief (可选) 存储所有由该池管理（包括激活和非激活）的Actor实例，方便统一销毁。*/
	UPROPERTY()
	TArray<AAfterImageActor*> AllManagedActors;

	/**
	 * @brief 尝试扩展对象池，生成一个新的Actor实例。
	 * @return 返回新生成的Actor指针，如果已达到MaxPoolSize或生成失败，则返回 nullptr。
	 */
	AAfterImageActor* TryGrowPool();

    /** @brief 记录当前池中的总Actor数量 (包括激活和非激活) */
    int32 CurrentTotalPooledActors = 0;
};