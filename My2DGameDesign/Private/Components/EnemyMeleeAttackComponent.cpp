// My2DGameDesign/Private/Components/EnemyMeleeAttackComponent.cpp
#include "Components/EnemyMeleeAttackComponent.h"
#include "DataAssets/Enemy/EnemyMeleeAttackSettingsDA.h"
#include "Enemies/EnemyCharacterBase.h" // 需要基类来获取接口
#include "Enemies/EvilCreature.h"       // 需要具体类来获取碰撞体组件
#include "Components/PrimitiveComponent.h" // 需要基础碰撞体类
#include "Interfaces/AnimationListenerProvider/EnemySpecificAnimListenerProvider.h"
#include "Interfaces/AnimationListener/EnemyMeleeAttackAnimListener.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
// #include "Kismet/GameplayStatics.h" // 如果需要播放声音等

UEnemyMeleeAttackComponent::UEnemyMeleeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

	bCanAttack = true;
	bIsAttacking = false;
    ActiveCollisionShapeName = NAME_None;
}

void UEnemyMeleeAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemyCharacter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyMeleeAttackComponent '%s' requires its owner to be derived from AEnemyCharacterBase!"), *GetName());
        SetActive(false);
		return;
	}

    if(!AttackSettings)
    {
         UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent '%s' on Actor '%s' is missing AttackSettings Data Asset!"), *GetName(), *OwnerEnemyCharacter->GetName());
    }
}

void UEnemyMeleeAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清理可能仍在运行的计时器
    if(GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
        GetWorld()->GetTimerManager().ClearTimer(ActiveCollisionTimerHandle); // 清理碰撞计时器
    }
	Super::EndPlay(EndPlayReason);
}


bool UEnemyMeleeAttackComponent::ExecuteAttack(AActor* Target)
{
    // --- 前置检查 ---
    if (!bCanAttack || bIsAttacking || !AttackSettings || !OwnerEnemyCharacter.IsValid())
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("EnemyMeleeAttackComponent '%s': Executing Melee Attack."), *GetName()); // 不再强调目标

    // --- 开始攻击流程 ---
    bIsAttacking = true; // 标记整个攻击流程开始（直到冷却结束）
    bCanAttack = false;
    BeginAttackSwing();    // 清空本次挥砍的命中记录
    StartAttackCooldown(); // 启动冷却

    // --- 通知动画实例 ---
    TScriptInterface<IEnemyMeleeAttackAnimListener> Listener = GetAnimListener();
    if (Listener)
    {
        // Target 仍然可以传递给动画实例，用于视觉或其他逻辑（例如，攻击时更精确地朝向目标）
        Listener->Execute_OnMeleeAttackStarted(Listener.GetObject(), Target);
    }
     else
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent '%s': Could not get valid MeleeAnimListener from owner."), *GetName());
    }

    // 注意：我们不再需要在这里存储 CurrentTarget 了

    return true; // 攻击成功开始
}

void UEnemyMeleeAttackComponent::ActivateMeleeCollision(FName ShapeIdentifier, float Duration)
{
	// 获取 Owner Actor
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || Duration <= 0) return;

	// --- 通过接口获取碰撞体 ---
	UPrimitiveComponent* ShapeToActivate = nullptr;
	// 尝试将 Owner 转换为 IMeleeShapeProvider 接口
	if (IMeleeShapeProvider* ShapeProvider = Cast<IMeleeShapeProvider>(OwnerActor))
	{
		// 如果 Owner 实现了接口，调用接口函数获取形状
		ShapeToActivate = IMeleeShapeProvider::Execute_GetMeleeShapeComponent(OwnerActor, ShapeIdentifier);
	}
	else
	{
		// Owner 没有实现接口，无法获取形状
		UE_LOG(LogTemp, Warning, TEXT("ActivateMeleeCollision: Owner '%s' does not implement IMeleeShapeProvider."), *OwnerActor->GetName());
		return; // 无法继续
	}
	// --- 接口使用结束 ---

	if (ShapeToActivate) // 检查接口是否返回了有效的形状
	{
		UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Activating collision shape '%s' for %.2f seconds."), *ShapeIdentifier.ToString(), Duration);
		BeginAttackSwing();
		ShapeToActivate->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ActiveCollisionShapeName = ShapeIdentifier;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(ActiveCollisionTimerHandle);
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this, FName("DeactivateMeleeCollision"), ShapeIdentifier);
			GetWorld()->GetTimerManager().SetTimer(ActiveCollisionTimerHandle, TimerDelegate, Duration, false);
		}
	}
	else
	{
		// 接口返回了 nullptr，说明 Owner (实现了接口) 没有提供这个标识符对应的形状
		UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent: Owner '%s' (IMeleeShapeProvider) did not provide a shape for identifier '%s'."), *OwnerActor->GetName(), *ShapeIdentifier.ToString());
	}
}


void UEnemyMeleeAttackComponent::DeactivateMeleeCollision(FName ShapeIdentifier)
{
     if (!OwnerEnemyCharacter.IsValid() || ShapeIdentifier == NAME_None || ShapeIdentifier != ActiveCollisionShapeName) return; // 确保关闭的是当前激活的

     AEvilCreature* OwnerCreature = Cast<AEvilCreature>(OwnerEnemyCharacter.Get());
     if(!OwnerCreature) return; // 理论上不会发生，因为激活时检查过

     UPrimitiveComponent* ShapeToDeactivate = OwnerCreature->GetMeleeHitShapeComponent(ShapeIdentifier);

	if (ShapeToDeactivate)
	{
        // 确保计时器确实指向了这个函数调用，清除计时器句柄
         if (GetWorld())
         {
            GetWorld()->GetTimerManager().ClearTimer(ActiveCollisionTimerHandle);
         }

        UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Deactivating collision shape '%s'."), *ShapeIdentifier.ToString());
		ShapeToDeactivate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ActiveCollisionShapeName = NAME_None; // 清除激活记录

        // 在这里不清空 HitActorsThisSwing，BeginAttackSwing 会在下次激活时清空
	}
}


void UEnemyMeleeAttackComponent::StartAttackCooldown()
{
	if (AttackSettings && AttackSettings->AttackCooldown > 0 && GetWorld())
	{
        // 防止重复启动冷却计时器
         if (!GetWorld()->GetTimerManager().IsTimerActive(AttackCooldownTimer))
         {
            GetWorld()->GetTimerManager().SetTimer(
                AttackCooldownTimer,
                this,
                &UEnemyMeleeAttackComponent::OnAttackCooldownFinished,
                AttackSettings->AttackCooldown,
                false
            );
         }
	}
	else
	{
		OnAttackCooldownFinished(); // 立即结束冷却
	}
}

void UEnemyMeleeAttackComponent::OnAttackCooldownFinished()
{
    UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent '%s': Cooldown finished."), *GetName());
	bCanAttack = true;
    bIsAttacking = false; // 整个攻击流程结束
    // 清空可能残留的激活碰撞体状态
    if(ActiveCollisionShapeName != NAME_None)
    {
        DeactivateMeleeCollision(ActiveCollisionShapeName);
    }
}

TScriptInterface<IEnemyMeleeAttackAnimListener> UEnemyMeleeAttackComponent::GetAnimListener() const
{
    if (OwnerEnemyCharacter.IsValid())
	{
		if (IEnemySpecificAnimListenerProvider* Provider = Cast<IEnemySpecificAnimListenerProvider>(OwnerEnemyCharacter.Get()))
		{
			TScriptInterface<IEnemyMeleeAttackAnimListener> Listener = Provider->Execute_GetMeleeAttackAnimListener(OwnerEnemyCharacter.Get());
			if (Listener)
			{
				return Listener;
			}
		}
	}
	return nullptr;
}

void UEnemyMeleeAttackComponent::BeginAttackSwing()
{
    // 清空本次攻击命中的目标记录
    HitActorsThisSwing.Empty();
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Cleared HitActorsThisSwing for new swing."));
}

// HandleDamageApplication 函数现在不需要了
/*
void UEnemyMeleeAttackComponent::HandleDamageApplication()
{
    // ... (旧逻辑移除) ...
}
*/