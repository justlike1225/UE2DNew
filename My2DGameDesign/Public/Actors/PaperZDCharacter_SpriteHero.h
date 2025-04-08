// My2DGameDesign/Public/PaperZDCharacter_SpriteHero.h
#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h" // 包含输入值结构体
#include "PaperZDCharacter.h" // 包含基类
#include "Interfaces/ActionInterruptSource.h"
#include "Interfaces/HeroAnimationStateProvider.h"
#include "Interfaces/FacingDirectionProvider.h"
#include "UObject/ScriptInterface.h" // 包含 TScriptInterface
#include "PaperZDCharacter_SpriteHero.generated.h"

// --- 前向声明 ---
class UPaperZDAnimInstance;
class UHeroCombatComponent;
class UDashComponent;
class UAfterimageComponent;
class UCameraComponent;
class UBoxComponent;
class UCapsuleComponent; // 需要包含它，因为仍然有定义
class UInputMappingContext;
class UInputAction;
class UPaperFlipbookComponent;
class ICharacterAnimationStateListener; // 前向声明动画状态监听器接口

// 声明一个新的多播委托，没有参数
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionInterruptSignature);

/**
 * 玩家控制的2D英雄角色类，基于PaperZDCharacter。
 * 集成了移动、动画以及通过组件实现的核心能力（冲刺、战斗、残影）。
 */
UCLASS()
class MY2DGAMEDESIGN_API APaperZDCharacter_SpriteHero : public APaperZDCharacter,public IFacingDirectionProvider, public IActionInterruptSource,   // <-- 新增
                                                         public IHeroAnimationStateProvider  // <-- 新增
{
    GENERATED_BODY()

public:
  
    // 构造函数
    APaperZDCharacter_SpriteHero();

    // --- IFacingDirectionProvider 接口实现 ---
    // BlueprintNativeEvent 需要一个 _Implementation 函数在 C++ 中实现
    virtual FVector GetFacingDirection_Implementation() const override;
    /** 当一个高优先级动作（如跳跃、冲刺）将要开始，可能会打断其他动作时广播 */
    UPROPERTY(BlueprintAssignable, Category = "Character|Events")
    FOnActionInterruptSignature OnActionWillInterrupt;
    // --- 组件 Getters ---
    UFUNCTION(BlueprintPure, Category = "Components")
    UDashComponent* GetDashComponent() const { return DashComponent; }

    UFUNCTION(BlueprintPure, Category = "Components")
    UAfterimageComponent* GetAfterimageComponent() const { return AfterimageComponent; }

    UFUNCTION(BlueprintPure, Category = "Components")
    UHeroCombatComponent* GetHeroCombatComponent() const { return CombatComponent; }

    
    // IActionInterruptSource
    virtual void BroadcastActionInterrupt_Implementation() override; // 注意是 _Implementation

    // IAnimationStateProvider
    virtual TScriptInterface<ICharacterAnimationStateListener> GetAnimStateListener_Implementation() const override; // 注意是 _Implementation


    // --- 状态查询 ---
    /** 判断角色是否正在根据输入尝试行走 */
    UFUNCTION(BlueprintPure, Category = "Movement")
    bool IsWalking() const { return bIsWalking; }

    /** 判断角色是否正在根据输入尝试奔跑 */
    UFUNCTION(BlueprintPure, Category = "Movement")
    bool IsRunning() const { return bIsRunning; }

    


protected:
    // --- 组件 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAfterimageComponent> AfterimageComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDashComponent> DashComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UHeroCombatComponent> CombatComponent;

   

    // --- 摄像机 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> Camera;

    // --- 输入配置 ---
    /** 主要的玩家输入映射上下文 */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> PlayerMappingContext;

    /** 跳跃输入动作 */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    /** 移动输入动作 (左右) */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    /** 奔跑输入动作 (通常是修改键) */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> RunAction;

    // --- 移动参数 ---
    /** 行走时的最大速度 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float WalkSpeed = 100.f;

    /** 奔跑时的最大速度 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float RunSpeed = 300.f;

    // --- 内部状态变量 ---
    /** 角色是否可以执行跳跃 (通常在地面时为 true) */
    bool bIsCanJump = false;
    /** 角色是否按下了移动键 */
    bool bIsWalking = false;
    /** 角色是否按下了奔跑键 (且在移动) */
    bool bIsRunning = false;

    // --- 新增：缓存实现了监听器接口的动画实例 ---
    /** 缓存实现了 ICharacterAnimationStateListener 接口的动画实例 */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess = "true"))
    TScriptInterface<ICharacterAnimationStateListener> AnimationStateListener;


    // --- 生命周期函数与重写函数 ---
    virtual void BeginPlay() override;
    virtual void NotifyControllerChanged() override; // 处理控制器附加，添加输入映射
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // 设置输入绑定
    virtual void Landed(const FHitResult& Hit) override; // 角色落地时调用
    virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
                                                  const FVector& PreviousFloorContactNormal,
                                                  const FVector& PreviousLocation, float TimeDelta) override; // 角色从边缘掉落时调用
    

    // --- 输入处理函数 (现在需要调用接口推送状态) ---
    /** 处理跳跃输入开始 */
    void OnJumpStarted(const FInputActionValue& Value);
    /** 处理跳跃输入结束 */
    void OnJumpCompleted(const FInputActionValue& Value);
    /** 处理奔跑输入触发 (按住) */
    void OnRunTriggered(const FInputActionValue& Value);
    /** 处理奔跑输入结束 (松开) */
    void OnRunCompleted(const FInputActionValue& Value);
    /** 处理移动输入触发 (按住) */
    void OnMoveTriggered(const FInputActionValue& Value);
    /** 处理移动输入结束 (松开) */
    void OnMoveCompleted(const FInputActionValue& Value);


    // --- 响应组件委托的函数 ---
    /** 当 DashComponent 发出 OnDashStarted 信号时调用 */
    UFUNCTION()
    void HandleDashStarted();
    /** 当 DashComponent 发出 OnDashEnded 信号时调用 */
    UFUNCTION()
    void HandleDashEnded();


    // --- 初始化辅助函数 ---
    /** 初始化移动相关的参数 */
    void InitializeMovementParameters();
   
    /** 创建并设置摄像机组件 */
    void SetupCamera();
   
    /** 根据输入方向设置角色的 Sprite 朝向 */
    void SetDirection(float Direction)  const;

};