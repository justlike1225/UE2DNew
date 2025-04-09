// My2DGameDesign/Private/Enemies/EvilCreature.cpp
#include "Enemies/EvilCreature.h"
#include "Components/EnemyMeleeAttackComponent.h"
#include "Components/TeleportComponent.h"
#include "Components/CapsuleComponent.h" // <-- 包含胶囊体头文件
#include "PaperFlipbookComponent.h"    // <-- 需要获取 Sprite 用于附加
#include "Interfaces/Damageable.h"       // <-- 需要包含可受击接口
#include "DataAssets/Enemy/EnemyMeleeAttackSettingsDA.h" // <-- 需要获取伤害设置

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

        // 绑定碰撞事件处理函数
        MeleeHit1->OnComponentBeginOverlap.AddDynamic(this, &AEvilCreature::HandleMeleeHit);
    }

	// 其他构造函数逻辑...
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

    // 可以在这里再次检查 MeleeHit1 是否正确附加（如果构造函数中附加到 RootComponent）
    if (MeleeHit1 && GetSprite() && MeleeHit1->GetAttachParent() != GetSprite())
    {
        MeleeHit1->AttachToComponent(GetSprite(), FAttachmentTransformRules::KeepRelativeTransform);
         UE_LOG(LogTemp, Log, TEXT("AEvilCreature BeginPlay: Re-attached MeleeHit1 to Sprite Component."));
    }

    // 确保忽略自身的碰撞
    if(MeleeHit1)
    {
        MeleeHit1->IgnoreActorWhenMoving(this, true);
    }
}


// 实现 GetMeleeHitShapeComponent 函数
UCapsuleComponent* AEvilCreature::GetMeleeHitShapeComponent(FName ShapeIdentifier) const
{
    // 当前只有一个碰撞体，直接返回它，忽略 ShapeIdentifier (但保留参数方便扩展)
    if (ShapeIdentifier == EvilCreatureAttackShapeNames::Melee1)
    {
        return MeleeHit1;
    }
   /*// 如果未来添加了其他形状，可以在这里添加判断
    else if (ShapeIdentifier == EvilCreatureAttackShapeNames::Melee2)
    {
        return MeleeHitShapeHeavy; // 假设有 MeleeHitShapeHeavy
    }*/

    UE_LOG(LogTemp, Warning, TEXT("AEvilCreature::GetMeleeHitShapeComponent: Could not find shape with identifier '%s'."), *ShapeIdentifier.ToString());
    return nullptr;
}

// 实现 HandleMeleeHit 函数
void AEvilCreature::HandleMeleeHit(
    UPrimitiveComponent* OverlappedComponent, // 这个就是 MeleeHit1
    AActor* OtherActor,                       // 被碰到的 Actor
    UPrimitiveComponent* OtherComp,           // 被碰到的 Actor 的组件
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // --- 安全检查 ---
    if (!OtherActor || OtherActor == this || !MeleeAttackComponent || !MeleeAttackComponent->IsAttacking())
    {
        // 1. 确保碰到的不是自己
        // 2. 确保攻击组件有效
        // 3. 确保当前确实处于攻击状态 (防止碰撞体残留或误触)
        return;
    }

    // --- 防止重复命中 ---
    // 检查这个 Actor 在本次攻击挥砍中是否已经被命中过
    if (MeleeAttackComponent->HitActorsThisSwing.Contains(OtherActor))
    {
        return; // 已经打过了，不再处理
    }

    // --- 检查目标是否可受击 ---
    if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
    {
        // --- 记录命中 ---
        MeleeAttackComponent->HitActorsThisSwing.Add(OtherActor); // 记录下来，防止重复命中

        // --- 获取伤害值和攻击者信息 ---
        float DamageToApply = 0.0f;
        if (MeleeAttackComponent->AttackSettings) // 确保数据资产有效
        {
            DamageToApply = MeleeAttackComponent->AttackSettings->AttackDamage;
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("HandleMeleeHit: MeleeAttackComponent on %s is missing AttackSettings DA! Applying 0 damage."), *GetName());
        }

        AController* InstigatorController = GetController(); // 获取自身的控制器
        AActor* DamageCauser = this; // 伤害来源是自己

        // --- 施加伤害 ---
        UE_LOG(LogTemp, Log, TEXT("EvilCreature '%s' melee hit '%s' for %.1f damage."), *GetName(), *OtherActor->GetName(), DamageToApply);
        IDamageable::Execute_ApplyDamage(OtherActor, DamageToApply, DamageCauser, InstigatorController, SweepResult);

        // --- (可选) 播放命中特效/声音 ---
        // if(MeleeAttackComponent->AttackSettings->HitEffect) { ... }
        // if(MeleeAttackComponent->AttackSettings->HitSound) { ... }
    }
    // else: 如果碰到的 Actor 不能接受伤害 (比如墙壁或其他敌人)，则忽略
}