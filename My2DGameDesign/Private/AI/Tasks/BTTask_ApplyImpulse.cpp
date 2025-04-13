// 文件路径: Private/AI/Tasks/BTTask_ApplyImpulse.cpp
#include "AI/Tasks/BTTask_ApplyImpulse.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h" // <--- 包含黑板组件头文件
#include "GameFramework/Actor.h"           // <--- 包含 Actor 头文件

// 构造函数
UBTTask_ApplyImpulse::UBTTask_ApplyImpulse()
{
	NodeName = "Apply Impulse Towards Target"; // 更新节点名称

	// 重要：告知编辑器 TargetActorKey 期望的是一个 Actor 类型的 Object
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ApplyImpulse, TargetActorKey), AActor::StaticClass());
}

// 核心执行逻辑
EBTNodeResult::Type UBTTask_ApplyImpulse::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1. 获取 AI 控制器和 Pawn
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	// 2. 获取移动组件
	ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn);
	UCharacterMovementComponent* MovementComponent = ControlledCharacter ? ControlledCharacter->GetCharacterMovement() : nullptr;
	if (!MovementComponent) return EBTNodeResult::Failed;

	// 3. 获取黑板组件和目标 Actor
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent(); // 使用 OwnerComp 获取黑板
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ApplyImpulse: BlackboardComponent is missing."));
		return EBTNodeResult::Failed;
	}

	// 从黑板获取目标 Actor 对象
	UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(TargetObject); // 尝试转换为 Actor 指针

	// 检查目标 Actor 是否有效
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ApplyImpulse: Target Actor is null or invalid in Blackboard key '%s'."), *TargetActorKey.SelectedKeyName.ToString());
		return EBTNodeResult::Failed; // 没有有效目标，任务失败
	}

	// 4. 计算冲量方向 (从 Pawn 指向 Target)
	FVector DirectionToTarget = TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation();

	// (推荐) 对于 2D 游戏，通常只关心水平方向的冲刺，忽略 Z 轴差异
	FVector ImpulseDirection = DirectionToTarget.GetSafeNormal2D(); // 获取 2D 方向并标准化

	// 确保方向向量是有效的 (非零)
	if (ImpulseDirection.IsNearlyZero())
	{
		// 如果目标和自身位置重合或非常接近，方向可能为零
		UE_LOG(LogTemp, Log, TEXT("BTTask_ApplyImpulse: Direction to target is nearly zero (maybe overlapping?). Skipping impulse."));
		// 在这种情况下，可以选择直接成功，因为无法施加有效冲量
		return EBTNodeResult::Succeeded;
		// 或者返回 Failed，取决于你希望的行为
		// return EBTNodeResult::Failed;
	}

	// 5. 计算最终的冲量向量
	FVector FinalImpulse = ImpulseDirection * ImpulseStrength;

	// 6. 施加冲量
	MovementComponent->AddImpulse(FinalImpulse, bVelocityChange);
	UE_LOG(LogTemp, Log, TEXT("BTTask_ApplyImpulse: Applied impulse %s towards Target %s for Pawn %s"), *FinalImpulse.ToString(), *TargetActor->GetName(), *ControlledPawn->GetName());

	// 7. 任务完成
	return EBTNodeResult::Succeeded;
}

// 更新编辑器描述
FString UBTTask_ApplyImpulse::GetStaticDescription() const
{
	// 在节点名称后添加目标黑板键的信息
	FString KeyDesc = TargetActorKey.SelectedKeyName.ToString();
	return FString::Printf(TEXT("%s\nTarget Key: '%s'\nStrength: %.1f"), *Super::GetStaticDescription(), *KeyDesc, ImpulseStrength);
	// 或者直接返回固定文本，因为主要信息是类名和参数
	// return FString::Printf(TEXT("Apply Impulse Towards Target: '%s' (Strength: %.1f)"), *KeyDesc, ImpulseStrength);
}