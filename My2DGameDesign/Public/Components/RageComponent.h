#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RageComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRageChangedSignature, float, NewRage, float, MaxRage);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MY2DGAMEDESIGN_API URageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URageComponent();
	UFUNCTION(BlueprintCallable, Category="Rage")
	void RestoreRage(float NewRage, float NewMaxRage);
	/**
	 * @brief 尝试增加怒气值。
	 * @param Amount 要增加的怒气量。
	 */
	UFUNCTION(BlueprintCallable, Category = "Rage System")
	void AddRage(float Amount);
	/**
	   * @brief 尝试消耗指定数量的怒气。
	   * @param AmountToConsume 要消耗的怒气量。
	   * @return 如果当前怒气足够消耗，则返回 true，否则返回 false。
	   */
	UFUNCTION(BlueprintCallable, Category = "Rage System")
	bool ConsumeRage(float AmountToConsume);
	/** 获取当前怒气值 */
	UFUNCTION(BlueprintPure, Category = "Rage System")
	float GetCurrentRage() const { return CurrentRage; }

	/** 获取最大怒气值 */
	UFUNCTION(BlueprintPure, Category = "Rage System")
	float GetMaxRage() const { return MaxRage; }

	/** 当怒气值发生变化时广播 */
	UPROPERTY(BlueprintAssignable, Category = "Rage System | Events")
	FOnRageChangedSignature OnRageChanged;

protected:
	virtual void BeginPlay() override;

	/** 最大怒气值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rage System | Configuration", meta = (ClampMin = "0.1"))
	float MaxRage = 100.0f;

	/** 每次攻击命中敌人增加的怒气值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rage System | Configuration", meta = (ClampMin = "0.0"))
	float RageGainOnHit = 10.0f;

	/** 当前怒气值 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Rage System | Status", meta = (AllowPrivateAccess = "true"))
	float CurrentRage = 0.0f;

};