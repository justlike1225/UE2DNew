#include "AI/BST/BTService_UpdateFacingDirection.h" // 包含对应的头文件 (路径可能需要根据你的项目调整)
#include "BehaviorTree/BlackboardComponent.h"   // 需要访问黑板
#include "AIController.h"                     // 需要获取AI控制器
#include "GameFramework/Actor.h"              // 需要 Actor 类
#include "Enemies/EnemyCharacterBase.h"       // 需要调用 EnemyCharacterBase 的 SetFacingDirection 函数

#include "Math/UnrealMathUtility.h"           // 需要 FMath::Abs 和 KINDA_SMALL_NUMBER

// 构造函数实现
UBTService_UpdateFacingDirection::UBTService_UpdateFacingDirection()
{
	NodeName = "Update Facing Direction"; // 在行为树编辑器中显示的节点名称

	// --- 服务执行频率设置 ---
	// Interval: 服务 TickNode 函数调用的基本时间间隔（秒）
	Interval = 0.15f; // 例如，每 0.15 秒检查一次朝向
	// RandomDeviation: 给 Interval 增加一个随机变化量，避免所有AI在同一帧执行，稍微错开负载
	RandomDeviation = 0.05f; // 实际间隔会在 (Interval - RandomDeviation) 到 (Interval + RandomDeviation) 之间随机
}

// TickNode 函数实现 - 核心逻辑
void UBTService_UpdateFacingDirection::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                float DeltaSeconds)
{
	// 首先调用父类的 TickNode，这是一个好习惯
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// --- 获取必要的对象引用 ---
	AAIController* AIController = OwnerComp.GetAIOwner(); // 获取拥有此行为树的 AI 控制器
	if (!AIController)
	{
		
		return;
	}

	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent(); // 获取黑板组件
	if (!BlackboardComp)
	{
		return;
	}

	
	APawn* ControlledPawnRaw = AIController->GetPawn();
	if (!ControlledPawnRaw)
	{
		return;
	}

	
	AEnemyCharacterBase* ControlledPawn = Cast<AEnemyCharacterBase>(ControlledPawnRaw);
	if (!ControlledPawn)
	{
		return;
	}

	
	UObject* TargetObject = BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(TargetObject);
	
	if (TargetActor)
	{
		
		FVector SelfLocation = ControlledPawn->GetActorLocation();
		FVector TargetLocation = TargetActor->GetActorLocation();

		
		float DirectionX = TargetLocation.X - SelfLocation.X;

	
		if (FMath::Abs(DirectionX) > KINDA_SMALL_NUMBER)
		{
			
			bool bWantsToFaceRight = (DirectionX > 0.0f);

			
			ControlledPawn->SetFacingDirection(bWantsToFaceRight);
		}
	}
	
}
