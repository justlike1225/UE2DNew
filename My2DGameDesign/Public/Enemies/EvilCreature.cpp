// My2DGameDesign/Private/Enemies/EvilCreature.cpp
#include "Enemies/EvilCreature.h"
#include "Components/EnemyMeleeAttackComponent.h"
#include "Components/TeleportComponent.h"
#include "Components/CapsuleComponent.h" // <-- 包含胶囊体头文件
#include "PaperFlipbookComponent.h"    // <-- 需要获取 Sprite 用于附加


AEvilCreature::AEvilCreature()
{
	// --- 创建攻击和传送组件 ---
	MeleeAttackComponent = CreateDefaultSubobject<UEnemyMeleeAttackComponent>(TEXT("MeleeAttackComponent"));
	TeleportComponent = CreateDefaultSubobject<UTeleportComponent>(TEXT("TeleportComponent"));

    // --- 创建近战攻击碰撞体 ---
    MeleeHit1 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeHit1"));
    if (MeleeHit1)
    {
        // 将碰撞体附加到角色的 Sprite 上 (或者武器插槽，如果使用骨骼动画)
        // 确保 GetSprite() 返回的是有效的组件
        if(GetSprite())
        {
             MeleeHit1->SetupAttachment(GetSprite());
        }
         else
         {
             // 如果 Sprite 还未创建（可能发生在基类构造之后），可以附加到 RootComponent
             // 但更好的做法是确保 Sprite 在此之前已创建或在 BeginPlay 中再尝试附加
             MeleeHit1->SetupAttachment(RootComponent);
             UE_LOG(LogTemp, Warning, TEXT("AEvilCreature Constructor: Could not find Sprite Component yet, attached MeleeHit1 to RootComponent temporarily."));
         }

        // 设置碰撞体的大小和相对位置 (这些值需要根据你的角色和攻击动画调整!)
        MeleeHit1->SetRelativeLocation(FVector(-14.0f, 0.0f, 19.0f)); // 示例位置：在角色前方
        MeleeHit1->SetCapsuleHalfHeight(30.0f);
        MeleeHit1->SetCapsuleRadius(8.0f);
        // 设置碰撞体的旋转
        MeleeHit1->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f)); // 默认朝向
        // 设置碰撞配置
        MeleeHit1->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 默认关闭碰撞
        MeleeHit1->SetCollisionProfileName(TEXT("OverlapOnlyPawn")); // 或者你自定义的Profile，只检测 Pawn
        MeleeHit1->SetCollisionResponseToAllChannels(ECR_Ignore);
        MeleeHit1->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 只与 Pawn 产生 Overlap 事件
        MeleeHit1->CanCharacterStepUpOn = ECB_No;
        MeleeHit1->SetGenerateOverlapEvents(true); // 确保生成 Overlap 事件

        
    }

}

UPrimitiveComponent* AEvilCreature::GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const
{
    // 根据标识符返回对应的碰撞体组件
    if (ShapeIdentifier == EvilCreatureAttackShapeNames::Melee1)
    {
        return MeleeHit1; // MeleeHit1 是我们在 AEvilCreature 中定义的 UCapsuleComponent*
    }
    // else if (ShapeIdentifier == ...) return OtherShape; // 如果有其他形状

    // 如果标识符无效或未找到，返回 nullptr
    return nullptr;
}


void AEvilCreature::BeginPlay()
{
    Super::BeginPlay();
    if (MeleeHit1 && GetSprite() && MeleeHit1->GetAttachParent() != GetSprite())
    {
        MeleeHit1->AttachToComponent(GetSprite(), FAttachmentTransformRules::KeepRelativeTransform);
        UE_LOG(LogTemp, Log, TEXT("AEvilCreature BeginPlay: Re-attached MeleeHit1 to Sprite Component."));
    }
    if(MeleeHit1)
    {
        MeleeHit1->IgnoreActorWhenMoving(this, true);
    }
}


