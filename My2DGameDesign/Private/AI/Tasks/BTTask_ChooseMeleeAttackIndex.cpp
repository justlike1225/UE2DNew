// My2DGameDesign/Private/AI/Tasks/BTTask_ChooseMeleeAttackIndex.cpp
#include "AI/Tasks/BTTask_ChooseMeleeAttackIndex.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h" // 需要随机数

UBTTask_ChooseMeleeAttackIndex::UBTTask_ChooseMeleeAttackIndex()
{
	NodeName = "Choose Melee Attack Index";
	// 指定我们要写入的黑板键 (通过 BlackboardKey 属性，继承自 UBTTask_BlackboardBase)
	BlackboardKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ChooseMeleeAttackIndex, MeleeAttackIndexKey));
}

EBTNodeResult::Type UBTTask_ChooseMeleeAttackIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	if (MaxAttackIndex < MinAttackIndex) // 基本检查
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ChooseMeleeAttackIndex: MaxAttackIndex (%d) is less than MinAttackIndex (%d). Defaulting to Min."), MaxAttackIndex, MinAttackIndex);
		BlackboardComp->SetValueAsInt(BlackboardKey.SelectedKeyName, MinAttackIndex);
		return EBTNodeResult::Succeeded;
	}

	// 在 Min 和 Max 之间随机选择一个整数 (包含两者)
	int32 ChosenIndex = UKismetMathLibrary::RandomIntegerInRange(MinAttackIndex, MaxAttackIndex);

	// 将选择的索引写入黑板
	BlackboardComp->SetValueAsInt(BlackboardKey.SelectedKeyName, ChosenIndex);

	UE_LOG(LogTemp, Verbose, TEXT("BTTask_ChooseMeleeAttackIndex: Set MeleeAttackIndex to %d"), ChosenIndex);

	// 任务立即成功完成
	return EBTNodeResult::Succeeded;
}