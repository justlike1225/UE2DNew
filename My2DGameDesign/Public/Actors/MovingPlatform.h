
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"

class UPaperSpriteComponent;
class UStaticMeshComponent; // 或者 UPaperSpriteComponent
class UBoxComponent;
class UInterpToMovementComponent;

UCLASS(Blueprintable) // Blueprintable 允许创建蓝图子类
class MY2DGAMEDESIGN_API AMovingPlatform : public AActor
{
	GENERATED_BODY()

public:
	AMovingPlatform();

protected:
	virtual void BeginPlay() override;

	// 平台的主要碰撞体，玩家站立的地方
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> PlatformCollisionComp; // 使用 Box 作为示例


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> VisualComp;


	// 处理移动的核心组件
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components") // BlueprintReadWrite 允许蓝图修改其属性
	TObjectPtr<UInterpToMovementComponent> InterpMovementComp;

public:
	// (可选) 允许蓝图或其他代码手动控制移动
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StartMovement();

	UFUNCTION(BlueprintCallable, Category="Movement")
	void StopMovement();

	UFUNCTION(BlueprintCallable, Category="Movement")
	void ResetMovement();
};