// HazardousActorBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PaperSpriteComponent.h"
#include "HazardousActorBase.generated.h"

class APaperZDCharacter_SpriteHero;

UCLASS(Abstract, Blueprintable)
class MY2DGAMEDESIGN_API AHazardousActorBase : public AActor
{
    GENERATED_BODY()

public:
    AHazardousActorBase();

    /** Blueprint/C++ 子类需实现，返回碰撞组件 */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hazard")
    UPrimitiveComponent* GetHazardCollisionComponent() const;
    virtual UPrimitiveComponent* GetHazardCollisionComponent_Implementation() const;

protected:
    virtual void BeginPlay() override;

    /** 子类返回的碰撞组件 */
    UPROPERTY(Transient)
    UPrimitiveComponent* CollisionComp;

    /** 精灵渲染组件 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPaperSpriteComponent* SpriteComp;

    // --- 危险区属性 & 逻辑 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard Settings")
    float DamageAmount = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard Settings", meta=(ClampMin="0.01"))
    float DamageInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard Settings")
    bool bDamageOnInitialOverlap = true;

    TWeakObjectPtr<APaperZDCharacter_SpriteHero> OverlappingHeroPtr;
    FTimerHandle DamageTimerHandle;

    UFUNCTION()
    virtual void OnHazardOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                      bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnHazardOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    virtual void ApplyPeriodicDamage();
};

