
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AfterimageComponent.generated.h"

class UAfterImagePoolComponent;
class UHeroFXSettingsDA;
class AAfterImageActor; 
class UPaperFlipbookComponent;
class UPaperFlipbook;
class UMaterialInterface;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MY2DGAMEDESIGN_API UAfterimageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAfterimageComponent();

protected:
	/** @brief 用于配置特效（包括残影）参数的数据资产实例。*/
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UHeroFXSettingsDA> FXSettings;
	/** @brief 指向管理残影Actor的对象池组件。会在BeginPlay时尝试查找。*/
	UPROPERTY(Transient) // Transient表示这个属性不需要保存或加载
	TObjectPtr<UAfterImagePoolComponent> AfterImagePool;
	// 内部使用的变量
	float CurrentAfterImageInterval = 0.05f;
	float CurrentAfterImageLifetime = 0.3f;
	FName CurrentOpacityParamName = FName("Opacity");
	float CurrentInitialOpacity = 0.5f;
	float CurrentFadeUpdateInterval = 0.03f;
	
	// 用于生成残影的Actor类
	UPROPERTY(EditDefaultsOnly, Category = "Afterimage Effect")
	TSubclassOf<AAfterImageActor> AfterImageClass;

	// 残影材质
	UPROPERTY(EditDefaultsOnly, Category = "Afterimage Effect")
	UMaterialInterface* AfterImageMaterial;

	
	// 生成残影的时间间隔，单位为秒
	UPROPERTY(EditDefaultsOnly, Category = "Afterimage Effect", meta = (ClampMin = "0.01"))
	float AfterImageInterval = 0.05f;

	
	// 残影的生存时间，单位为秒
	UPROPERTY(EditDefaultsOnly, Category = "Afterimage Effect", meta = (ClampMin = "0.01"))
	float AfterImageLifetime = 0.3f;

	
	// 用于控制残影生成定时的句柄
	FTimerHandle AfterImageSpawnTimer;

    
    // 标识是否正在生成残影
    bool bIsSpawning = false;

    // 弱引用，指向拥有该组件的Actor的UPaperFlipbookComponent
    UPROPERTY() 
    TWeakObjectPtr<UPaperFlipbookComponent> OwnerSpriteComponent;


	virtual void BeginPlay() override;

	
	// 用于生成残影的函数
	UFUNCTION() 
	void SpawnAfterImage();

public:
	// 启动残影生成
	UFUNCTION(BlueprintCallable, Category = "Afterimage Effect")
	void StartSpawning();

	// 停止残影生成
	UFUNCTION(BlueprintCallable, Category = "Afterimage Effect")
	void StopSpawning();

    // 查询当前是否正在生成残影
    UFUNCTION(BlueprintPure, Category = "Afterimage Effect")
    bool IsSpawning() const { return bIsSpawning; }

};
