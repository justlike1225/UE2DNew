// HazardousActorBase.cpp
#include "Actors/HazardousActorBase.h"
#include "Interfaces/Damageable.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "TimerManager.h"
#include "Engine/World.h"

AHazardousActorBase::AHazardousActorBase()
{
    PrimaryActorTick.bCanEverTick = false;
    // 创建精灵组件
    SpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComp"));
    SpriteComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpriteComp->SetupAttachment(RootComponent);
}

UPrimitiveComponent* AHazardousActorBase::GetHazardCollisionComponent_Implementation() const
{
    UE_LOG(LogTemp, Error, TEXT("%s: GetHazardCollisionComponent 未被子类覆盖！"), *GetName());
    return nullptr;
}

void AHazardousActorBase::BeginPlay()
{
    Super::BeginPlay();

    // 获取子类提供的碰撞组件
    CollisionComp = GetHazardCollisionComponent();
    if (!CollisionComp)
    {
        UE_LOG(LogTemp, Error, TEXT("%s: 无有效碰撞组件，无法初始化 Hazard"), *GetName());
        return;
    }
  

    CollisionComp->SetGenerateOverlapEvents(true);
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AHazardousActorBase::OnHazardOverlapBegin);
    CollisionComp->OnComponentEndOverlap  .AddDynamic(this, &AHazardousActorBase::OnHazardOverlapEnd);
}

void AHazardousActorBase::OnHazardOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                               bool bFromSweep, const FHitResult& SweepResult)
{
    APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OtherActor);
    if (Hero && !OverlappingHeroPtr.IsValid())
    {
        OverlappingHeroPtr = Hero;
        if (bDamageOnInitialOverlap)
            ApplyPeriodicDamage();
        GetWorldTimerManager().SetTimer(DamageTimerHandle, this,
            &AHazardousActorBase::ApplyPeriodicDamage, DamageInterval, true,
            bDamageOnInitialOverlap ? DamageInterval : 0.f);
    }
}

void AHazardousActorBase::OnHazardOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APaperZDCharacter_SpriteHero* Hero = Cast<APaperZDCharacter_SpriteHero>(OtherActor);
    if (Hero && Hero == OverlappingHeroPtr.Get())
    {
        OverlappingHeroPtr.Reset();
        GetWorldTimerManager().ClearTimer(DamageTimerHandle);
    }
}

void AHazardousActorBase::ApplyPeriodicDamage()
{
    if (!OverlappingHeroPtr.IsValid())
    {
        GetWorldTimerManager().ClearTimer(DamageTimerHandle);
        return;
    }
    APaperZDCharacter_SpriteHero* Hero = OverlappingHeroPtr.Get();
    if (Hero->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
    {
        FHitResult Hit;
        IDamageable::Execute_ApplyDamage(Hero, DamageAmount, this, nullptr, Hit);
    }
}
