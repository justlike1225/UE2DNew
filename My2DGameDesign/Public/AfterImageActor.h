// My2DGameDesign/Public/AfterImageActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AfterImageActor.generated.h"

class UAfterImagePoolComponent;
class UPaperFlipbookComponent;
class UPaperFlipbook;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class AfterImagePoolComponent; // <--- 前向声明对象池组件

UCLASS()
class MY2DGAMEDESIGN_API AAfterImageActor : public AActor
{
	GENERATED_BODY()

public:
	AAfterImageActor();

	// --- 对象池接口 ---

	/**
	 * @brief 激活残影Actor，设置其状态并开始生命周期。替代原有的 Initialize + SetLifeSpan。
	 * @param FlipbookToCopy 要显示的Flipbook
	 * @param MaterialToUse 应用的材质 (会创建动态实例)
	 * @param LifeTime 效果持续时间 (秒)
	 * @param SpriteTransform Actor的世界变换
	 * @param InOpacityParamName 材质中控制透明度的参数名
	 * @param InInitialOpacity 初始透明度
	 * @param InFadeUpdateInterval 淡出效果更新频率
	 * @param InOwningPool 指向管理此Actor的对象池组件
	 */
	void Activate(
		UPaperFlipbook* FlipbookToCopy,
		UMaterialInterface* MaterialToUse,
		float LifeTime,
		const FTransform& SpriteTransform,
		FName InOpacityParamName,
		float InInitialOpacity,
		float InFadeUpdateInterval,
		UAfterImagePoolComponent* InOwningPool // <--- 新增：传入对象池引用
	);

	/**
	 * @brief 反激活Actor，将其重置为非活动状态，准备回收到对象池。
	 */
	void Deactivate();

	/** 返回该Actor当前是否处于激活状态 */
	bool IsActive() const { return bIsActive; }


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sprite")
	UPaperFlipbookComponent* AfterImageSprite;

	// UPROPERTY(EditDefaultsOnly, Category = "Sprite") // 这个可以去掉了，因为材质是从外部传入的
	// UMaterialInterface* AfterImageMaterialBase;

	UPROPERTY()
	UMaterialInstanceDynamic* MaterialInstanceDynamic = nullptr;

	// --- 状态变量 ---
	FTimerHandle FadeTimerHandle;       // 控制淡出更新
	FTimerHandle LifetimeTimerHandle;   // 控制总生命周期结束
	float CreationTime = 0.0f;         // Actor激活时的时间
	float ActorLifeTime = 0.0f;        // 配置的生命周期
	FName OpacityParameterName;        // 透明度参数名
	float CurrentInitialOpacity = 1.0f; // 初始透明度 (给个默认值)
	bool bIsActive = false;            // 标记Actor是否激活

	// 指向拥有它的对象池 (弱指针防止循环引用，虽然这里可能不是严格必须，但是好习惯)
	UPROPERTY()
	TWeakObjectPtr<UAfterImagePoolComponent> OwningPoolPtr;

	// --- 内部函数 ---
	UFUNCTION()
	void UpdateFade(); // 更新淡出效果

	UFUNCTION()
	void OnLifetimeExpired(); // 生命周期结束时调用，触发Deactivate

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ResetActor(); // 重置Actor状态的辅助函数
};