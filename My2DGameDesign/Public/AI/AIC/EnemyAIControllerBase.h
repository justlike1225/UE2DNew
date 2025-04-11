// My2DGameDesign/Public/AI/EnemyAIControllerBase.h

#pragma once

#include "CoreMinimal.h"
#include "AIController.h" // 继承自引擎的 AIController

#include "Perception/AIPerceptionTypes.h" // 需要用到 FAIStimulus 等感知类型
#include "EnemyAIControllerBase.generated.h" // 生成的头文件

// --- 前向声明 ---
class UBehaviorTreeComponent;   // 行为树组件
class UBlackboardComponent; // 黑板组件
class UAIPerceptionComponent; // AI 感知组件
class UBehaviorTree;        // 行为树资产

/**
 * 敌人 AI 控制器的基类
 */
UCLASS()
class MY2DGAMEDESIGN_API AEnemyAIControllerBase : public AAIController
{
	GENERATED_BODY() // UE类宏

public:
	// 构造函数
	AEnemyAIControllerBase(const FObjectInitializer& ObjectInitializer);

	// --- Team Interface ---
	/** 设置队伍 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Team")
	FGenericTeamId TeamId = FGenericTeamId(1); // 默认敌人队伍为 1

	/** 实现获取队伍 ID 的接口函数 */
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	/** 实现获取对其他队伍态度的接口函数 */
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	// --- End Team Interface ---
    /** 黑板键名常量：用于存储目标 Actor (通常是玩家) 的键名 */
    static const FName TargetActorKeyName;
    /** 黑板键名常量：用于存储是否能看到目标的布尔值的键名 */
    static const FName CanSeeTargetKeyName;
    /** 黑板键名常量：用于存储 SelfActor 的键名 */
    static const FName SelfActorKeyName;


protected:
	// 当此控制器控制 (Possess) 一个 Pawn 时调用
	virtual void OnPossess(APawn* InPawn) override;
	// 当此控制器解除对 Pawn 的控制 (UnPossess) 时调用
	virtual void OnUnPossess() override;

	// --- AI 核心组件 ---

	/** AI 感知组件，用于处理视觉、听觉等感知信息 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	/** 行为树组件，负责执行行为树逻辑 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	/** 黑板组件，用于存储 AI 的记忆和状态数据 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

    // --- 运行时变量 ---

    /** 指向当前运行的行为树资产（通常从被控制的 Pawn 获取）*/
	UPROPERTY(Transient) // Transient 表示这个变量不需要保存，因为它在运行时设置
    TObjectPtr<UBehaviorTree> EnemyBehaviorTree;

    // --- 事件处理函数 ---

    /**
     * UFUNCTION() 标记此函数可以绑定到委托。
     * 当 AIPerceptionComponent 感知到或丢失 Actor 时，会调用此函数。
     * @param Actor 被感知到的 Actor。
     * @param Stimulus 感知刺激的信息 (比如是视觉、听觉，以及是否成功感知到)。
     */
    UFUNCTION()
    virtual void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // --- 内部辅助函数 ---

    /**
     * 安全地在黑板上设置或清除目标 Actor 的值。
     * @param TargetActor 要设置的目标 Actor，如果为 nullptr 则清除该键值。
     */
    virtual void SetTargetActorOnBlackboard(AActor* TargetActor);
};