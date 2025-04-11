// My2DGameDesign/Public/Components/TeleportComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeleportComponent.generated.h"

// 前向声明
class UTeleportSettingsDA;
class AEnemyCharacterBase;
class IEnemyTeleportAnimListener; // 需要监听器接口
template <class InterfaceType>
class TScriptInterface; // 需要 TScriptInterface

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MY2DGAMEDESIGN_API UTeleportComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTeleportComponent();

	/**
	 * @brief 尝试执行传送。通常由 AI 行为树任务调用。
	 * @param TargetLocation 期望传送到的目标位置。
	 * @return 如果成功开始传送流程（不在冷却中且不在传送中），则返回 true。
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Ability | Teleport")
	bool ExecuteTeleport(const FVector& TargetLocation);

	/** 查询当前是否可以执行传送 (主要检查冷却) */
	UFUNCTION(BlueprintPure, Category = "Enemy Ability | Teleport | Status")
	bool CanTeleport() const { return bCanTeleport; }

	/** 查询当前是否正在执行传送 */
	UFUNCTION(BlueprintPure, Category = "Enemy Ability | Teleport | Status")
	bool IsTeleporting() const { return bIsTeleporting; }

	// --- 由动画通知或其他逻辑调用 ---
	/** @brief 在传送准备动画的特定帧（或延迟后）调用，实际执行位置改变。 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Ability | Teleport | Internal")
	void PerformActualTeleport();

	/** @brief 在传送结束动画的特定帧（或延迟后）调用，标记传送状态结束。 */
	UFUNCTION(BlueprintCallable, Category = "Enemy Ability | Teleport | Internal")
	void FinishTeleportState();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** @brief 指向配置此传送能力的数据资产。*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport | Configuration")
	TObjectPtr<UTeleportSettingsDA> TeleportSettings;

	// --- 内部状态 ---
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Teleport | Status", meta=(AllowPrivateAccess="true"))
	bool bCanTeleport = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Teleport | Status", meta=(AllowPrivateAccess="true"))
	bool bIsTeleporting = false;

	/** 存储 ExecuteTeleport 时传入的目标位置，供 PerformActualTeleport 使用 */
	FVector PendingTeleportLocation;

	// --- 计时器 ---
	FTimerHandle TeleportCooldownTimer;
	FTimerHandle TeleportCastTimer; // 用于延迟执行实际传送

	// --- 内部引用 ---
	UPROPERTY(Transient)
	TWeakObjectPtr<AEnemyCharacterBase> OwnerEnemyCharacter;

private:
	/** 启动传送冷却 */
	void StartTeleportCooldown();

	/** 传送冷却结束时调用 */
	UFUNCTION()
	void OnTeleportCooldownFinished();

	/** 获取动画监听器接口 */
	TScriptInterface<IEnemyTeleportAnimListener> GetAnimListener() const;

	/** 内部辅助，用于安全传送 */
	bool TrySetActorLocation(const FVector& NewLocation);
};
