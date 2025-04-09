// My2DGameDesign/Private/Components/TeleportComponent.cpp
#include "Components/TeleportComponent.h"
#include "DataAssets/Enemy/TeleportSettingsDA.h"
#include "Enemies/EnemyCharacterBase.h"
#include "Interfaces/AnimationListenerProvider/EnemySpecificAnimListenerProvider.h" // 需要 Provider 接口
#include "Interfaces/AnimationListener/EnemyTeleportAnimListener.h"          // 需要 Listener 接口
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h" // 可能需要暂时禁用移动
// #include "Kismet/GameplayStatics.h" // 如果需要播放特效声音

UTeleportComponent::UTeleportComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

	bCanTeleport = true;
	bIsTeleporting = false;
	PendingTeleportLocation = FVector::ZeroVector;
}

void UTeleportComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemyCharacter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("TeleportComponent '%s' requires owner derived from AEnemyCharacterBase!"), *GetName());
		SetActive(false); // 禁用组件
		return;
	}

	if (!TeleportSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportComponent '%s' on Actor '%s' is missing TeleportSettings Data Asset!"), *GetName(), *OwnerEnemyCharacter->GetName());
		// 可以选择禁用或使用默认值
	}
}

void UTeleportComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理所有计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TeleportCooldownTimer);
		GetWorld()->GetTimerManager().ClearTimer(TeleportCastTimer);
	}
	Super::EndPlay(EndPlayReason);
}

bool UTeleportComponent::ExecuteTeleport(const FVector& TargetLocation)
{
	// --- 前置检查 ---
	if (!bCanTeleport || bIsTeleporting || !TeleportSettings || !OwnerEnemyCharacter.IsValid() || !GetWorld())
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("TeleportComponent '%s': Executing Teleport towards %s."), *GetName(), *TargetLocation.ToString());

	// --- 更新状态 ---
	bIsTeleporting = true;
	bCanTeleport = false;
	PendingTeleportLocation = TargetLocation; // 存储目标位置

	// --- 通知动画实例 ---
	TScriptInterface<IEnemyTeleportAnimListener> Listener = GetAnimListener();
	if (Listener)
	{
		Listener->Execute_OnTeleportStateChanged(Listener.GetObject(), true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportComponent '%s': Could not get valid TeleportAnimListener from owner."), *GetName());
	}

	// --- 启动冷却 ---
	StartTeleportCooldown();

	// --- (可选) 播放开始特效/声音 ---
	// if (TeleportSettings->TeleportStartEffect) { UGameplayStatics::SpawnEmitterAttached(...); }
	// if (TeleportSettings->TeleportStartSound) { UGameplayStatics::PlaySoundAtLocation(...); }

    // --- (可选) 暂时禁用移动 ---
    // if (OwnerEnemyCharacter->GetCharacterMovement())
    // {
    //     OwnerEnemyCharacter->GetCharacterMovement()->DisableMovement();
    // }

	// --- 设置延迟执行实际传送 ---
	if (TeleportSettings->TeleportCastTime > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TeleportCastTimer,
			this,
			&UTeleportComponent::PerformActualTeleport,
			TeleportSettings->TeleportCastTime,
			false
		);
	}
	else // 如果没有准备时间，立即执行
	{
		PerformActualTeleport();
	}

	return true;
}

void UTeleportComponent::PerformActualTeleport()
{
	if (!bIsTeleporting || !OwnerEnemyCharacter.IsValid())
	{
		return; // 如果状态已改变或 Owner 无效，则中止
	}

	UE_LOG(LogTemp, Log, TEXT("TeleportComponent '%s': Performing actual teleport to %s."), *GetName(), *PendingTeleportLocation.ToString());

    // 尝试设置位置
	bool bTeleported = TrySetActorLocation(PendingTeleportLocation);

    if (bTeleported)
    {
        // --- (可选) 播放结束特效/声音 ---
        // 注意：如果结束特效/声音应该在角色出现在新位置后播放，放在这里
        // if (TeleportSettings->TeleportEndEffect) { UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TeleportSettings->TeleportEndEffect, PendingTeleportLocation); }
        // if (TeleportSettings->TeleportEndSound) { UGameplayStatics::PlaySoundAtLocation(GetWorld(), TeleportSettings->TeleportEndSound, PendingTeleportLocation); }

        // 你可以选择在这里直接结束传送状态，或者依赖动画通知
        // 如果你的传送动画包含一个明显的“出现”动作，最好等动画播放到那个点再结束状态
        // FinishTeleportState(); // 如果是瞬移效果，可以在这里调用
    }
    else // 传送失败
    {
        UE_LOG(LogTemp, Warning, TEXT("TeleportComponent '%s': Failed to set actor location during teleport. Ending state."), *GetName());
        FinishTeleportState(); // 传送失败也应该结束状态
    }
}

void UTeleportComponent::FinishTeleportState()
{
	if (!bIsTeleporting) return; // 避免重复调用

	UE_LOG(LogTemp, Log, TEXT("TeleportComponent '%s': Finishing teleport state."), *GetName());

	bIsTeleporting = false;

    // --- 重新启用移动 ---
    // if (OwnerEnemyCharacter.IsValid() && OwnerEnemyCharacter->GetCharacterMovement())
    // {
    //     OwnerEnemyCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking); // 或之前的模式
    // }

	// --- 通知动画实例 ---
	TScriptInterface<IEnemyTeleportAnimListener> Listener = GetAnimListener();
	if (Listener)
	{
		Listener->Execute_OnTeleportStateChanged(Listener.GetObject(), false);
	}
}


void UTeleportComponent::StartTeleportCooldown()
{
	if (TeleportSettings && TeleportSettings->TeleportCooldown > 0 && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TeleportCooldownTimer,
			this,
			&UTeleportComponent::OnTeleportCooldownFinished,
			TeleportSettings->TeleportCooldown,
			false
		);
	}
	else
	{
		OnTeleportCooldownFinished(); // 立即结束冷却
	}
}

void UTeleportComponent::OnTeleportCooldownFinished()
{
	bCanTeleport = true;
	UE_LOG(LogTemp, Verbose, TEXT("TeleportComponent '%s': Cooldown finished."), *GetName());
}

TScriptInterface<IEnemyTeleportAnimListener> UTeleportComponent::GetAnimListener() const
{
	if (OwnerEnemyCharacter.IsValid())
	{
		// 通过 Provider 获取 Listener
		if (IEnemySpecificAnimListenerProvider* Provider = Cast<IEnemySpecificAnimListenerProvider>(OwnerEnemyCharacter.Get()))
		{
			TScriptInterface<IEnemyTeleportAnimListener> Listener = Provider->Execute_GetTeleportAnimListener(OwnerEnemyCharacter.Get());
			if (Listener) // 检查有效性
			{
				return Listener;
			}
		}
	}
	return nullptr; // 返回无效接口
}

bool UTeleportComponent::TrySetActorLocation(const FVector& NewLocation)
{
    if (OwnerEnemyCharacter.IsValid())
    {
        // 使用 TeleportTo 更安全，它会处理碰撞
        return OwnerEnemyCharacter->TeleportTo(NewLocation, OwnerEnemyCharacter->GetActorRotation(), false, false);
        // 或者，如果需要无视碰撞的瞬移：
        // return OwnerEnemyCharacter->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }
    return false;
}