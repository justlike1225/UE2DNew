
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/InputBindingComponent.h" // <-- 包含接口头文件
#include "DashComponent.generated.h"

class UHeroDashSettingsDA;
class ACharacter;
class UCharacterMovementComponent;
class UAfterimageComponent;
class UPaperFlipbookComponent;
struct FInputActionValue;
class UInputAction; // <-- 前向声明输入动作类
class UEnhancedInputComponent; // <-- 前向声明增强输入组件

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashEndedSignature);
// --- 让组件继承 IInputBindingComponent ---
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MY2DGAMEDESIGN_API UDashComponent : public UActorComponent, public IInputBindingComponent
{
	GENERATED_BODY()

public:
	UDashComponent();
	// 冲刺开始时广播
	UPROPERTY(BlueprintAssignable, Category = "Dash|Events")
	FOnDashStartedSignature OnDashStarted_Event; //改名以区分之前的内部委托

	// 冲刺结束时广播
	UPROPERTY(BlueprintAssignable, Category = "Dash|Events")
	FOnDashEndedSignature OnDashEnded_Event;
	// --- 从 Character 移过来的输入动作资产 ---
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DashAction; // <-- 在这里定义 DashAction

	// --- 接口函数的声明 ---
	virtual void BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent) override;

	// --- 其他成员 ... ---
	UPROPERTY(BlueprintAssignable, Category = "Dash|Events")
	FOnDashStateChangedSignature OnDashStarted;
	UPROPERTY(BlueprintAssignable, Category = "Dash|Events")
	FOnDashStateChangedSignature OnDashEnded;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UHeroDashSettingsDA> DashSettings;
	
	UFUNCTION() 
	void HandleDashInputTriggered(const FInputActionValue& Value); 


	void PerformDash();
	UFUNCTION()
	void EndDash();
	UFUNCTION()
	void ResetDashCooldown();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	

	UFUNCTION(BlueprintPure, Category = "Dash|State")
	bool IsDashing() const { return bIsDashing; }
	UFUNCTION(BlueprintPure, Category = "Dash|State")
	bool CanDash() const { return bCanDash && !bIsDashing; }

private:
    // --- 原来的 HandleDashInput 逻辑现在可能在 HandleDashInputTriggered 里 ---
    void ExecuteDashLogic(); // 提取核心冲刺逻辑，避免 HandleDashInputTriggered 过长

    // --- 其他私有成员 ... ---
    float CurrentDashSpeed = 1500.f;
    float CurrentDashDuration = 0.2f;
    float CurrentDashCooldown = 1.0f;
    FTimerHandle DashEndTimer;
    FTimerHandle DashCooldownTimer;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dash|State", meta=(AllowPrivateAccess = "true"))
    bool bIsDashing = false;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dash|State", meta=(AllowPrivateAccess = "true"))
    bool bCanDash = true;
    float DashDirectionMultiplier = 1.0f;
    UPROPERTY()
    TWeakObjectPtr<ACharacter> OwnerCharacter;
    UPROPERTY()
    TWeakObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
    UPROPERTY()
    TWeakObjectPtr<UAfterimageComponent> OwnerAfterimageComponent;
    
    float OriginalGroundFriction = 8.0f;
    float OriginalMaxWalkSpeed = 600.0f;
    float OriginalGravityScale = 1.0f;
};
