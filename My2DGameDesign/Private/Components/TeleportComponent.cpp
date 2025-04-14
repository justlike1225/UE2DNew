// 文件路径: My2DGameDesign/Private/Components/TeleportComponent.cpp

#include "Components/TeleportComponent.h"
#include "DataAssets/Enemy/TeleportSettingsDA.h"
#include "Enemies/EnemyCharacterBase.h"
#include "Interfaces/AnimationListenerProvider/EnemySpecificAnimListenerProvider.h"
#include "Interfaces/AnimationListener/EnemyTeleportAnimListener.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h" // 包含移动组件头文件

UTeleportComponent::UTeleportComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // 通常不需要 Tick
	SetIsReplicatedByDefault(false); // 假设是单机或 AI 控制，不需要复制

	bCanTeleport = true;
	bIsTeleporting = false;
	PendingTeleportLocation = FVector::ZeroVector;
}

void UTeleportComponent::BeginPlay()
{
	Super::BeginPlay();

	// 获取拥有者引用
	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemyCharacter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("TeleportComponent owned by non-AEnemyCharacterBase or null owner! Deactivating."));
		// 禁用组件，防止后续出错
		SetActive(false);
		SetComponentTickEnabled(false);
		return;
	}

	// 检查数据资产
	if (!TeleportSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportComponent on %s is missing TeleportSettingsDA! Using defaults or potentially failing."), *GetOwner()->GetName());
		// 这里可以考虑设置一些硬编码的默认值，或者依赖调用方检查
	}
}

void UTeleportComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理计时器，防止在销毁后触发
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TeleportCooldownTimer);
		GetWorld()->GetTimerManager().ClearTimer(TeleportCastTimer);
	}
	Super::EndPlay(EndPlayReason);
}

bool UTeleportComponent::ExecuteTeleport(const FVector& TargetLocation)
{
	// 检查前置条件：是否可传送、是否正在传送、配置是否有效、拥有者是否有效、世界是否存在
	if (!bCanTeleport || bIsTeleporting || !TeleportSettings || !OwnerEnemyCharacter.IsValid() || !GetWorld())
	{
		// 添加日志说明具体原因
		if (!bCanTeleport) UE_LOG(LogTemp, Log, TEXT("TeleportComponent::ExecuteTeleport: Failed - Cannot teleport (Cooldown or disabled)."));
		if (bIsTeleporting) UE_LOG(LogTemp, Log, TEXT("TeleportComponent::ExecuteTeleport: Failed - Already teleporting."));
		if (!TeleportSettings) UE_LOG(LogTemp, Warning, TEXT("TeleportComponent::ExecuteTeleport: Failed - TeleportSettingsDA is missing."));
		if (!OwnerEnemyCharacter.IsValid()) UE_LOG(LogTemp, Error, TEXT("TeleportComponent::ExecuteTeleport: Failed - Invalid OwnerEnemyCharacter."));
		if (!GetWorld()) UE_LOG(LogTemp, Error, TEXT("TeleportComponent::ExecuteTeleport: Failed - World is null."));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("TeleportComponent::ExecuteTeleport: Initiating teleport to %s for %s."), *TargetLocation.ToString(), *GetOwner()->GetName());

	// 设置状态
	bIsTeleporting = true;
	bCanTeleport = false; // 进入冷却
	PendingTeleportLocation = TargetLocation;

	// 通知动画实例开始传送动画
	TScriptInterface<IEnemyTeleportAnimListener> Listener = GetAnimListener();
	if (Listener)
	{
		// 注意 BlueprintNativeEvent 需要使用 Execute_ 前缀
		Listener->Execute_OnTeleportStateChanged(Listener.GetObject(), true);
		UE_LOG(LogTemp, Verbose, TEXT("TeleportComponent: Notified AnimInstance OnTeleportStateChanged(true)."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportComponent on %s could not find AnimListener."), *GetOwner()->GetName());
	}

	// 启动冷却计时器
	StartTeleportCooldown();

	// 处理传送准备时间 (Cast Time)
	if (TeleportSettings->TeleportCastTime > KINDA_SMALL_NUMBER) // 使用 KINDA_SMALL_NUMBER 避免浮点误差
	{
		UE_LOG(LogTemp, Verbose, TEXT("TeleportComponent: Starting cast timer (%.2fs)."), TeleportSettings->TeleportCastTime);
		// 设置计时器，在准备时间结束后调用 PerformActualTeleport
		GetWorld()->GetTimerManager().SetTimer(
			TeleportCastTimer,
			this,
			&UTeleportComponent::PerformActualTeleport,
			TeleportSettings->TeleportCastTime,
			false // 不循环
		);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("TeleportComponent: No cast time, performing teleport immediately."));
		// 没有准备时间，立即执行传送
		PerformActualTeleport();
	}

	return true; // 传送流程已成功启动
}

void UTeleportComponent::PerformActualTeleport()
{
	// 再次检查状态，防止计时器触发时状态已改变
	if (!bIsTeleporting || !OwnerEnemyCharacter.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportComponent::PerformActualTeleport: Called while not teleporting or owner invalid. Aborting."));
		// 即使在这里中止，也应该确保状态被清理
		if (bIsTeleporting)
		{
			FinishTeleportState(); // 尝试清理状态
		}
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("TeleportComponent::PerformActualTeleport: Attempting to teleport %s to %s."), *GetOwner()->GetName(), *PendingTeleportLocation.ToString());

	// --- 核心修改点 ---
	// 尝试移动 Actor
	bool bTeleportedSuccessfully = TrySetActorLocation(PendingTeleportLocation);

	if (bTeleportedSuccessfully)
	{
		UE_LOG(LogTemp, Log, TEXT("TeleportComponent: Teleport successful for %s."), *GetOwner()->GetName());
		// **修改**: 无论传送动画是否播放完毕，只要物理移动成功，就立即结束传送的逻辑状态。
		FinishTeleportState(); // <--- 在成功后调用
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportComponent: Teleport failed for %s (TrySetActorLocation returned false)."), *GetOwner()->GetName());
		// 如果传送失败（例如目标位置无效），也要结束传送状态。
		FinishTeleportState(); // <--- 失败时原本就会调用，保持不变
	}
	// --- 核心修改点结束 ---
}

void UTeleportComponent::FinishTeleportState()
{
	// 检查是否真的处于传送状态，防止重复调用或意外调用
	if (!bIsTeleporting)
	{
		// UE_LOG(LogTemp, Verbose, TEXT("TeleportComponent::FinishTeleportState: Called but not currently teleporting. Ignoring."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("TeleportComponent::FinishTeleportState: Finishing teleport state for %s."), *GetOwner()->GetName());

	// 重置状态标志
	bIsTeleporting = false;

	// 通知动画实例结束传送动画
	TScriptInterface<IEnemyTeleportAnimListener> Listener = GetAnimListener();
	if (Listener)
	{
		Listener->Execute_OnTeleportStateChanged(Listener.GetObject(), false);
		UE_LOG(LogTemp, Verbose, TEXT("TeleportComponent: Notified AnimInstance OnTeleportStateChanged(false)."));
	}
	// 注意：冷却计时器是在 ExecuteTeleport 中启动的，不受 FinishTeleportState 影响。
}

void UTeleportComponent::StartTeleportCooldown()
{
	if (TeleportSettings && TeleportSettings->TeleportCooldown > 0 && GetWorld())
	{
		UE_LOG(LogTemp, Verbose, TEXT("TeleportComponent: Starting cooldown timer (%.2fs)."), TeleportSettings->TeleportCooldown);
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
		// 如果没有冷却时间，立即完成冷却
		OnTeleportCooldownFinished();
	}
}

void UTeleportComponent::OnTeleportCooldownFinished()
{
	UE_LOG(LogTemp, Log, TEXT("TeleportComponent: Cooldown finished for %s."), *GetOwner()->GetName());
	bCanTeleport = true;
}

TScriptInterface<IEnemyTeleportAnimListener> UTeleportComponent::GetAnimListener() const
{
	// 检查 Owner 是否仍然有效
	if (OwnerEnemyCharacter.IsValid())
	{
		// 尝试将 Owner 转换为动画监听器提供者接口
		// 注意：需要确保 AEnemyCharacterBase (或 AEvilCreature) 确实实现了 IEnemySpecificAnimListenerProvider
		if (IEnemySpecificAnimListenerProvider* Provider = Cast<IEnemySpecificAnimListenerProvider>(OwnerEnemyCharacter.Get()))
		{
			// 通过 Provider 接口获取特定的传送动画监听器
			TScriptInterface<IEnemyTeleportAnimListener> Listener = Provider->Execute_GetTeleportAnimListener(OwnerEnemyCharacter.Get());
			// 检查获取到的监听器是否有效 (既不是 nullptr 也不是无效的接口对象)
			if (Listener) // TScriptInterface 会自动处理空指针检查
			{
				return Listener;
			}
			else
			{
				// 如果 Provider 存在但返回了无效的 Listener
				UE_LOG(LogTemp, Warning, TEXT("TeleportComponent::GetAnimListener: Provider on %s returned an invalid TeleportAnimListener."), *GetOwner()->GetName());
			}
		}
		else
		{
			// 如果 Owner 没有实现 Provider 接口
			UE_LOG(LogTemp, Warning, TEXT("TeleportComponent::GetAnimListener: Owner %s does not implement IEnemySpecificAnimListenerProvider."), *GetOwner()->GetName());
		}
	}
	else
	{
		// 如果 Owner 无效
		UE_LOG(LogTemp, Error, TEXT("TeleportComponent::GetAnimListener: OwnerEnemyCharacter is invalid!"));
	}
	// 如果任何步骤失败，返回一个无效的接口
	return nullptr;
}

// 尝试安全地移动 Actor
bool UTeleportComponent::TrySetActorLocation(const FVector& NewLocation)
{
	if (OwnerEnemyCharacter.IsValid())
	{
		// 使用 AActor::TeleportTo。它比 SetActorLocation 更适合用于瞬移，
		// 可能会处理一些物理状态的重置。
		// 第三个参数 bIsATest 设为 false 表示实际执行传送。
		// 第四个参数 bNoCheck 设为 false 表示进行碰撞检测（如果目标位置有阻挡物，传送会失败）。
		// 如果你希望无论如何都强制移动过去（可能穿墙），可以将 bNoCheck 设为 true。
		// 但通常设为 false 更安全。
		bool bSuccess = OwnerEnemyCharacter->TeleportTo(NewLocation, OwnerEnemyCharacter->GetActorRotation(), false, false);
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("TeleportComponent::TrySetActorLocation: OwnerEnemyCharacter->TeleportTo failed (Location possibly obstructed)."));
		}
		return bSuccess;
	}
	UE_LOG(LogTemp, Error, TEXT("TeleportComponent::TrySetActorLocation: OwnerEnemyCharacter is invalid!"));
	return false;
}