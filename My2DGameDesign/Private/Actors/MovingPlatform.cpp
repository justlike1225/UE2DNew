// AMovingPlatform.cpp

#include "Actors/MovingPlatform.h"

#include "PaperSpriteComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h" // 或者 PaperSpriteComponent
#include "GameFramework/MovementComponent.h" // InterpTo 需要

#include "Components/InterpToMovementComponent.h"

AMovingPlatform::AMovingPlatform()
{
    PrimaryActorTick.bCanEverTick = false; // InterpToMovementComponent 自己处理 Tick
    // 1. 创建碰撞体并设为根
    PlatformCollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("PlatformCollision"));
    RootComponent = PlatformCollisionComp;

    // 配置碰撞体属性
    PlatformCollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic")); // 需要阻挡玩家 Pawn
    PlatformCollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 允许物理和查询
    PlatformCollisionComp->SetMobility(EComponentMobility::Movable); // 平台需要能移动
    PlatformCollisionComp->SetSimulatePhysics(false); // 通常平台不模拟物理，而是按路径移动
    PlatformCollisionComp->SetNotifyRigidBodyCollision(true); // 推荐开启，改善角色站立效果

    // 2. 创建视觉组件并附加到根 (碰撞体)
    VisualComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("VisualMesh"));
    // 如果用 Sprite: VisualComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("VisualSprite"));
    if (VisualComp)
    {
        VisualComp->SetupAttachment(RootComponent);
        VisualComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 视觉部分不参与碰撞
        VisualComp->SetMobility(EComponentMobility::Movable); // 确保视觉也能移动
    }

    // 3. 创建 InterpToMovementComponent
    InterpMovementComp = CreateDefaultSubobject<UInterpToMovementComponent>(TEXT("InterpMovement"));
    if (InterpMovementComp)
    {
        InterpMovementComp->SetUpdatedComponent(RootComponent); // 让它移动根组件
        InterpMovementComp->BehaviourType = EInterpToBehaviourType::PingPong; // 默认来回移动
        InterpMovementComp->Duration = 5.0f; // 默认完成一次单向移动需要5秒
        InterpMovementComp->bSweep = true; // 移动时进行扫描检测，防止穿墙
    }
}

void AMovingPlatform::BeginPlay()
{
    Super::BeginPlay();
    
  
}

void AMovingPlatform::StartMovement()
{
    if(InterpMovementComp)
    {
        InterpMovementComp->RestartMovement();
        InterpMovementComp->Activate(true);
    }
}

void AMovingPlatform::StopMovement()
{
     if(InterpMovementComp)
     {
        InterpMovementComp->StopMovementImmediately(); //或者 FinaliseMovement();
        InterpMovementComp->Deactivate();
     }
}

 void AMovingPlatform::ResetMovement()
{
     if(InterpMovementComp)
     {
         InterpMovementComp->ResetControlPoints(); // 可能需要重新设置路径点
         InterpMovementComp->RestartMovement(0.0f); // 从头开始
     }
}