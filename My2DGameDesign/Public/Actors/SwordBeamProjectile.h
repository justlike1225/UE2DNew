// My2DGameDesign/Public/Actors/SwordBeamProjectile.h (修正后)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwordBeamProjectile.generated.h"

// 前向声明
class UBoxComponent;
class UPaperSpriteComponent;
class UProjectileMovementComponent;
class UGameplayStatics;
class UDamageType;

UCLASS()
class MY2DGAMEDESIGN_API ASwordBeamProjectile : public AActor
{
    GENERATED_BODY()

public: // <--- 设为 Public
    ASwordBeamProjectile();

    // --- 组件 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPaperSpriteComponent> SpriteComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement; // <--- 现在是 Public

protected: // <--- Protected 成员放在这里   
    // --- 内部变量 ---
    UPROPERTY() // 用于存储从外部初始化的值
    float CurrentDamage = 0.0f;  

    // 缓存发射者的 Actor
    UPROPERTY()                       
    TWeakObjectPtr<AActor> InstigatorActor;


    // --- 生命周期函数和事件处理 ---
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnCollisionOverlapBegin(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult
    );

public: // <--- Public 函数
    /**
     * @brief 初始化抛射物 (由生成它的 CombatComponent 调用)
     * @param Direction 发射方向 (单位向量)
     * @param Speed 初始速度
     * @param Damage 基础伤害值
     * @param LifeSpan 生存时间 (秒)
     * @param Shooter 发射者 Actor
     */
    // *** 确认是 5 个参数 ***
    void InitializeProjectile(
        const FVector& Direction,
        float Speed,
        float Damage, 
        float LifeSpan,           
        AActor* Shooter
    );
};