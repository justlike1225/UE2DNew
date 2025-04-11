// My2DGameDesign/Private/AI/Services/BTService_UpdateCombatStatus.cpp
#include "AI/BSt/BTService_UpdateCombatStatus.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Enemies/EvilCreature.h" // 需要包含具体 Pawn 类来获取组件
#include "Components/HealthComponent.h"
#include "Components/EnemyMeleeAttackComponent.h"
#include "Components/TeleportComponent.h"
#include "Kismet/KismetSystemLibrary.h"
// #include "Kismet/KismetSystemLibrary.h" // 如果需要打印调试信息

UBTService_UpdateCombatStatus::UBTService_UpdateCombatStatus()
{
	NodeName = "Update Combat Status"; // 在行为树编辑器中显示的节点名称

	// 设置服务执行的频率 (可以根据需要调整)
	Interval = 0.2f;          // 每 0.2 秒执行一次 TickNode
	RandomDeviation = 0.05f;  // 增加一点随机性，避免所有 AI 同时 Tick
}

void UBTService_UpdateCombatStatus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// --- 日志点 1: 确认服务是否在 Ticking ---
	// UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateCombatStatus::TickNode Executing..."));

	// --- 获取必要的对象引用 ---
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();

	// --- 日志点 2: 检查关键对象是否有效 ---
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: BlackboardComponent is NULL."));
		return;
	}
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: AIController is NULL."));
		return;
	}

	AEvilCreature* ControlledPawn = Cast<AEvilCreature>(AIController->GetPawn());
	if (!ControlledPawn)
	{
		// 这个日志很重要，如果 Pawn 获取不到或类型不对，后面都没法进行
		UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: Controlled Pawn is NULL or not an AEvilCreature."));
		return;
	}
	// UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateCombatStatus: ControlledPawn: %s"), *ControlledPawn->GetName());


	// --- 获取组件 (添加检查) ---
	UHealthComponent* HealthComp = ControlledPawn->GetHealthComponent();
	UEnemyMeleeAttackComponent* MeleeComp = ControlledPawn->GetMeleeAttackComponent();
	UTeleportComponent* TeleportComp = ControlledPawn->GetTeleportComponent();
    if (!HealthComp) UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: HealthComp is NULL."));
    if (!MeleeComp) UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: MeleeComp is NULL."));
    if (!TeleportComp) UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: TeleportComp is NULL."));


	// --- 更新距离 ---
	// --- 日志点 3: 确认 TargetActorKey 是否设置正确 ---
	if (!TargetActorKey.IsSet())
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: TargetActorKey is not set in Behavior Tree Service details!"));
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));

	// --- 日志点 4: 检查获取到的 TargetActor ---
	if (TargetActor)
	{
		UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateCombatStatus: TargetActor found: %s at %s"),
			*TargetActor->GetName(),
			*TargetActor->GetActorLocation().ToString());
		UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateCombatStatus: ControlledPawn is at %s"),
			*ControlledPawn->GetActorLocation().ToString());

		float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());

		//--- 日志点 5: 检查计算出的距离 ---
		UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateCombatStatus: Calculated Distance: %.2f"), Distance);

		// --- 日志点 6: 确认 DistanceToTargetKey 是否设置正确 ---
        if (!DistanceToTargetKey.IsSet())
        {
            UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: DistanceToTargetKey is not set in Behavior Tree Service details!"));
        }
		BlackboardComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);

		//--- 日志点 7: 确认写入黑板后的值 (可选) ---
		float ReadbackDistance = BlackboardComp->GetValueAsFloat(DistanceToTargetKey.SelectedKeyName);
		UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateCombatStatus: Blackboard DistanceToTarget set to: %.2f"), ReadbackDistance);
	}
	else
	{
		// --- 日志点 8: 确认 TargetActor 获取失败 ---
		UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateCombatStatus: TargetActor is NULL. Clearing DistanceToTarget."));
		BlackboardComp->ClearValue(DistanceToTargetKey.SelectedKeyName);
	}

	// --- 更新攻击和传送就绪状态 (添加日志检查 Key 是否设置) ---
    if (CanMeleeAttackKey.IsSet())
    {
        bool bCanMelee = MeleeComp ? MeleeComp->CanAttack() : false;
	    BlackboardComp->SetValueAsBool(CanMeleeAttackKey.SelectedKeyName, bCanMelee);
    } else { UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: CanMeleeAttackKey not set!")); }

    if (CanTeleportKey.IsSet())
    {
        bool bCanTeleport = TeleportComp ? TeleportComp->CanTeleport() : false;
	    BlackboardComp->SetValueAsBool(CanTeleportKey.SelectedKeyName, bCanTeleport);
    } else { UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: CanTeleportKey not set!")); }


    // --- 更新低血量状态 (添加日志检查 Key 是否设置) ---
    if (IsHealthLowKey.IsSet())
    {
        if (HealthComp)
        {
            float CurrentHealth = HealthComp->GetCurrentHealth();
            float MaxHealth = HealthComp->GetMaxHealth();
            float Threshold = 0.3f; // 或者从黑板读
            bool bIsLow = (MaxHealth > 0 && (CurrentHealth / MaxHealth) <= Threshold);
            BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, bIsLow);
        }
        else
        {
            BlackboardComp->SetValueAsBool(IsHealthLowKey.SelectedKeyName, false);
        }
    } else { UE_LOG(LogTemp, Warning, TEXT("UBTService_UpdateCombatStatus: IsHealthLowKey not set!")); }


  // 打印最终调试信息 (这个可以一直开着，比较有用)
    FString TargetName = TargetActor ? TargetActor->GetName() : FString(TEXT("None"));
    FStringFormatNamedArguments Args;
    Args.Add(TEXT("Target"), TargetName);
    Args.Add(TEXT("Dist"), BlackboardComp->GetValueAsFloat(DistanceToTargetKey.SelectedKeyName));
    Args.Add(TEXT("CanMelee"), BlackboardComp->GetValueAsBool(CanMeleeAttackKey.SelectedKeyName) ? TEXT("T") : TEXT("F"));
    Args.Add(TEXT("CanTele"), BlackboardComp->GetValueAsBool(CanTeleportKey.SelectedKeyName) ? TEXT("T") : TEXT("F"));
    Args.Add(TEXT("LowHP"), BlackboardComp->GetValueAsBool(IsHealthLowKey.SelectedKeyName) ? TEXT("T") : TEXT("F"));
    FString DebugText = FString::Format(TEXT("Target: {Target}, Dist: {Dist}, Melee: {CanMelee}, Tele: {CanTele}, LowHP: {LowHP}"), Args);

   

  
     // 或者用 PrintString 输出到屏幕左上角
      UKismetSystemLibrary::PrintString(this, DebugText, true, false, FLinearColor::Yellow, 0.0f); // 持续时间0=单帧

     // 或者用 UE_LOG 输出到 Output Log
      UE_LOG(LogTemp, Log, TEXT("%s"), *DebugText);
}